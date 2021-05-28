#include "MCBot.h"
#include "PacketSender.h"

namespace mcbot
{

    static void print_winsock_error()
    {
        wchar_t* s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, WSAGetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&s, 0, NULL);
        printf("%S\n", s);
    }

    PacketSender::PacketSender(MCBot* bot)
    {
        this->bot = bot;
    }

    PacketSender::PacketSender()
    {
        this->bot = NULL;
    }

    int PacketSender::LoginMojang(std::string email, std::string password)
    {
        // Payload //
        char content_format[] =
            "{"
            "\"agent\": {"
            "\"name\": \"Minecraft\","
            "\"version\" : 1 },"
            "\"username\" : \"%s\","
            "\"password\" : \"%s\""
            "}\r\n";
        char content[256] = { 0 };
        sprintf_s(content, content_format, email.c_str(), password.c_str());

        // Send Payload //
        httplib::Client cli("https://authserver.mojang.com");
        auto response = cli.Post("/authenticate", content, "application/json");

        // Read Response //
        auto response_json = nlohmann::json::parse(response->body);
        std::string access_token = response_json["accessToken"];

        auto selected_profile = response_json["selectedProfile"];
        std::string username = selected_profile["name"];
        std::string uuid = selected_profile["id"];

        std::cout << "Logged into Mojang account:" << std::endl
            << "\tAccount resolved to " << username << std::endl;

        this->bot->SetSession(MojangSession(access_token, username, uuid));

        return 0;
    }

    int PacketSender::VerifyAccessToken()
    {
        // Payload //
        const char* content_format =
            "{"
            "\"accessToken\" : \"%s\""
            "}\r\n";

        std::string access_token = this->bot->GetSession().GetAccessToken();
        int content_length = strlen(content_format) + access_token.length();
        char* content = new char[content_length] {0};
        sprintf_s(content, content_length, content_format, access_token.c_str());

        // Send Payload //
        httplib::Client cli("https://authserver.mojang.com");
        auto response = cli.Post("/validate", content, "application/json");

        delete[] content;

        // If response is 204 then the access token is valid
        return response->status == 204 ? 0 : -1;
    }

    int PacketSender::SendSession(std::string server_id, std::string shared_secret, uint8_t* public_key, int public_key_length)
    {
        daft_hash_impl hasher;
        hasher.update((void*)server_id.c_str(), server_id.length());
        hasher.update((void*)shared_secret.c_str(), shared_secret.length());
        hasher.update((void*)public_key, public_key_length);

        std::string hash = hasher.finalise();


        // Payload //
        const char* content_format =
            "{"
            "\"accessToken\" : \"%s\","
            "\"selectedProfile\": \"%s\","
            "\"serverId\": \"%s\""
            "}\r\n";

        MojangSession session = this->bot->GetSession();
        int content_length = strlen(content_format) + session.GetAccessToken().length() + session.GetUUID().length() + hash.length();
        char* content = new char[content_length] {0};
        sprintf_s(content, content_length, content_format, session.GetAccessToken().c_str(), session.GetUUID().c_str(), hash.c_str());

        // Send Payload //
        httplib::Client cli("https://sessionserver.mojang.com");
        auto response = cli.Post("/session/minecraft/join", content, "application/json");

        delete[] content;

        // If response is 204 then session is valid
        return response->status == 204 ? 0 : -1;
    }

    void PacketSender::SendHandshake(char* hostname, unsigned short port)
    {
        this->bot->LogDebug(">>> Sending PacketHandshakingIn...");
        this->bot->SetState(State::HANDSHAKE);

        uint8_t packet[1028];
        size_t offset = 0;

        PacketEncoder::WriteVarInt(0x00, packet, offset); // packet id
        PacketEncoder::WriteVarInt(47, packet, offset);   // protocol version
        PacketEncoder::WriteString(hostname, packet, offset); // hostname
        PacketEncoder::WriteShort(port, packet, offset); // port
        PacketEncoder::WriteVarInt((int)State::LOGIN, packet, offset); // next state

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }
    }

    void PacketSender::SendLoginStart()
    {
        this->bot->LogDebug(">>> Sending PacketLoginInStart...");
        this->bot->SetState(State::LOGIN);

        uint8_t packet[1028];
        size_t offset = 0;

        PacketEncoder::WriteVarInt(0x00, packet, offset); // packet id
        PacketEncoder::WriteString((char*)this->bot->GetSession().GetUsername().c_str(), packet, offset); // username

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }
    }

    void PacketSender::SendEncryptionResponse(uint8_t* public_key, int public_key_length, uint8_t* verify_token, int verify_token_length, std::string shared_secret)
    {
        this->bot->LogDebug(">>> Sending PacketLoginInEncryptionBegin...");
        // Convert DER to PEM //
        char pem_format[] =
            "-----BEGIN PUBLIC KEY-----"
            "%s\n"
            "-----END PUBLIC KEY-----\n";

        char pem_text[1024] = { 0 };
        int max_encoded_length = Base64encode_len(public_key_length);
        int result = Base64encode(pem_text, (const char*)public_key, public_key_length);

        std::string temp;
        int i;
        for (i = 0; i < max_encoded_length; i++)
        {
            if (i % 64 == 0)
            {
                temp += '\n';
            }
            temp += pem_text[i];
        }

        int pem_formatted_size = strlen(pem_format) + temp.length() - 2;
        char* pem_formatted = new char[pem_formatted_size] {0};
        sprintf_s(pem_formatted, pem_formatted_size, pem_format, temp.c_str());

        // Get RSA corresponding to PEM
        BIO* bio = BIO_new_mem_buf((void*)pem_formatted, -1);
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        RSA* rsa_public_key = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
        if (rsa_public_key == NULL)
        {
            printf("ERROR: Could not load PUBLIC KEY! PEM_PacketDecoder::read_bio_RSA_PUBKEY FAILED: %s\n", ERR_error_string(ERR_get_error(), NULL));
        }
        BIO_free(bio);

        delete[] pem_formatted;

        this->bot->LogDebug("Public Key Info: ");
        BIO* keybio = BIO_new(BIO_s_mem());
        RSA_print(keybio, rsa_public_key, 0);
        char buffer[2048] = { 0 };
        std::string res = "";
        while (BIO_read(keybio, buffer, 2048) > 0)
        {
            this->bot->LogDebug(buffer);
        }
        BIO_free(keybio);

        unsigned char encrypted_shared_secret[256] = { 0 };
        int encrypted_shared_secret_len = RSA_public_encrypt(shared_secret.length(), (unsigned char*)shared_secret.c_str(), encrypted_shared_secret, rsa_public_key, RSA_PKCS1_PADDING);
        this->bot->LogDebug("Encrypted shared secret with public key");

        unsigned char encrypted_verify_token[256] = { 0 };
        int encrypted_verify_token_len = RSA_public_encrypt(verify_token_length, verify_token, encrypted_verify_token, rsa_public_key, RSA_PKCS1_PADDING);
        this->bot->LogDebug("Encrypted verify token with public key");

        uint8_t packet[1028];
        size_t offset = 0;

        PacketEncoder::WriteVarInt(0x01, packet, offset); // packet id

        PacketEncoder::WriteVarInt(encrypted_shared_secret_len, packet, offset); // shared secret length
        PacketEncoder::WriteByteArray(encrypted_shared_secret, encrypted_shared_secret_len, packet, offset); // shared secret

        PacketEncoder::WriteVarInt(encrypted_verify_token_len, packet, offset); // verify token length
        PacketEncoder::WriteByteArray(encrypted_verify_token, encrypted_verify_token_len, packet, offset); // verify token

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }
        else
        {
            this->bot->GetSocket().initialize_encryption((unsigned char*)shared_secret.c_str(), (unsigned char*)shared_secret.c_str());
            this->bot->LogDebug("Sent encryption response - ENCRYPTION ENABLED");
        }
    }

    void PacketSender::SendKeepAlive(int id)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInKeepAlive...");

        int packet_id = 0x00;
        uint8_t* packet = new uint8_t[PacketEncoder::GetVarIntNumBytes({ packet_id, id })];
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset); // packet id
        PacketEncoder::WriteVarInt(id, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;
    }

    void PacketSender::SendChatMessage(std::string message)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInChat...");

        int packet_id = 0x01;
        uint8_t* packet = new uint8_t[message.length() + PacketEncoder::GetVarIntNumBytes({ packet_id, (int)message.length() }) + 1]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WriteString(message, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }
        else
        {
            this->bot->LogDebug("Message: " + message);
        }

        delete[] packet;
    }

    void PacketSender::SendUseEntity(int entity_id, UseEntityType action)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInUseEntity...");

        int packet_id = 0x02;
        uint8_t* packet = new uint8_t[PacketEncoder::GetVarIntNumBytes({ packet_id, entity_id, (int)action })]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WriteVarInt(entity_id, packet, offset);
        PacketEncoder::WriteVarInt((int)action, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;
    }

    void PacketSender::SendPosition(Vector<double> position, bool on_ground)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInPosition...");


        int packet_id = 0x04;
        uint8_t* packet = new uint8_t[PacketEncoder::GetVarIntNumBytes(packet_id) + 3 * sizeof(double) + 1]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WriteDouble(position.GetX(), packet, offset);
        PacketEncoder::WriteDouble(position.GetY(), packet, offset);
        PacketEncoder::WriteDouble(position.GetZ(), packet, offset);
        PacketEncoder::WriteBoolean(on_ground, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;

        // Internal updates
        this->bot->GetPlayer().UpdateLocation(position);
    }

    void PacketSender::SendLook(float yaw, float pitch, bool on_ground)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInLook...");

        int packet_id = 0x05;
        uint8_t* packet = new uint8_t[PacketEncoder::GetVarIntNumBytes(packet_id) + 2 * sizeof(float) + 1]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WriteFloat(yaw, packet, offset);
        PacketEncoder::WriteFloat(pitch, packet, offset);
        PacketEncoder::WriteBoolean(on_ground, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;

        // Internal updates
        this->bot->GetPlayer().UpdateRotation(yaw, pitch);
    }

    void PacketSender::SendPositionLook(Vector<double> position, float yaw, float pitch, bool on_ground)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInPositionLook...");

        int packet_id = 0x06;
        uint8_t* packet = new uint8_t[PacketEncoder::GetVarIntNumBytes(packet_id) + 3 * sizeof(double) + 2 * sizeof(float) + 1]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WriteDouble(position.GetX(), packet, offset);
        PacketEncoder::WriteDouble(position.GetY(), packet, offset);
        PacketEncoder::WriteDouble(position.GetZ(), packet, offset);
        PacketEncoder::WriteFloat(yaw, packet, offset);
        PacketEncoder::WriteFloat(pitch, packet, offset);
        PacketEncoder::WriteBoolean(on_ground, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;

        // Internal updates
        this->bot->GetPlayer().UpdateLocation(position);
        this->bot->GetPlayer().UpdateRotation(yaw, pitch);
    }

    void PacketSender::SendBlockDig(DigStatus status, Vector<int> location, BlockFace face)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInBlockDig...");

        int packet_id = 0x07;
        uint8_t* packet = new uint8_t[PacketEncoder::GetVarIntNumBytes(packet_id) + 10]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WriteByte((uint8_t)status, packet, offset);
        PacketEncoder::WritePosition(location.GetX(), location.GetY(), location.GetZ(), packet, offset);
        PacketEncoder::WriteByte((uint8_t)face, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;
    }

    void PacketSender::SendBlockPlace(Vector<int> location, BlockFace face, Slot held_item, Vector<uint8_t> cursor_position)
    {
        /*this->bot->log_debug(">>> Sending PacketPlayInBlockPlace...");

        int packet_id = 0x08;
        uint8_t* packet = new uint8_t[PacketEncoder::get_var_int_size(packet_id) + 10]{ 0 };
        size_t offset = 0;

        PacketEncoder::write_var_int(packet_id, packet, offset);
        PacketEncoder::write_position(location.get_x(), location.get_y(), location.get_z(), packet, offset);
        PacketEncoder::write_byte((uint8_t)face, packet, offset);
        PacketEncoder::write_short()

        if (this->bot->get_socket().send_pack(packet, offset) <= 0)
        {
            this->bot->log_error("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;*/
    }

    void PacketSender::SendHeldItemSlot(short slot)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInHeldItemSlot...");

        int packet_id = 0x09;
        uint8_t* packet = new uint8_t[PacketEncoder::GetVarIntNumBytes(packet_id) + sizeof(short)]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WriteShort(slot, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;
    }

    void PacketSender::SendArmAnimation()
    {
        this->bot->LogDebug(">>> Sending PacketPlayInArmAnimation...");

        int packet_id = 0x0A;
        uint8_t* packet = new uint8_t[PacketEncoder::GetVarIntNumBytes(packet_id)]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;
    }

    void PacketSender::SendEntityAction(EntityAction action, int action_parameter)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInEntityAction...");

        int packet_id = 0x0B;
        uint8_t* packet = new uint8_t[PacketEncoder::GetVarIntNumBytes({ packet_id, (int)action, action_parameter, this->bot->GetPlayer().GetID() })]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WriteVarInt(this->bot->GetPlayer().GetID(), packet, offset);
        PacketEncoder::WriteVarInt((int)action, packet, offset);
        PacketEncoder::WriteVarInt(action_parameter, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;
    }


    void PacketSender::SendEntityAction(int player_id, EntityAction action, int param)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInEntityAction...");

        int packet_id = 0x0B;
        uint8_t* packet = new uint8_t[PacketEncoder::GetVarIntNumBytes({ packet_id, player_id, (int)action, param })]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WriteVarInt(player_id, packet, offset);
        PacketEncoder::WriteVarInt((int)action, packet, offset);
        PacketEncoder::WriteVarInt(param, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;
    }

    void PacketSender::SendSetCreativeSlot(short slot, Slot item)
    {
        //this->bot->log_debug(">>> Sending PacketPlayInSetCreativeSlot...");

        //int packet_id = 0x10;
        //uint8_t* packet = new uint8_t[1024]{ 0 };
        //size_t offset = 0;

        //PacketEncoder::write_var_int(packet_id, packet, offset);
        //PacketEncoder::write_short(slot, packet, offset);

        //if (this->bot->get_socket().send_pack(packet, offset) <= 0)
        //{
        //    this->bot->log_error("Failed to send packet");
        //    print_winsock_error();
        //}

        //delete[] packet;
    }

    void PacketSender::SendEnchantItem(uint8_t window_id, uint8_t enchantment)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInEnchantItem..");

        int packet_id = 0x11;
        uint8_t* packet = new uint8_t[1024]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WriteByte(window_id, packet, offset);
        PacketEncoder::WriteByte(enchantment, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;
    }

    void PacketSender::SendUpdateSign(Vector<int> location, std::string line1, std::string line2, std::string line3, std::string line4)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInUpdateSign...");

        int packet_id = 0x12;
        uint8_t* packet = new uint8_t[1024]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WritePosition(location.GetX(), location.GetY(), location.GetZ(), packet, offset);
        PacketEncoder::WriteString(line1, packet, offset);
        PacketEncoder::WriteString(line2, packet, offset);
        PacketEncoder::WriteString(line3, packet, offset);
        PacketEncoder::WriteString(line4, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;
    }

    void PacketSender::SendAbilities(uint8_t flags, float flying_speed, float walking_speed)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInAbilities...");

        int packet_id = 0x13;
        uint8_t* packet = new uint8_t[PacketEncoder::GetVarIntNumBytes(packet_id) + sizeof(uint8_t) + 2 * sizeof(float)]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WriteByte(flags, packet, offset);
        PacketEncoder::WriteFloat(flying_speed, packet, offset);
        PacketEncoder::WriteFloat(walking_speed, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;
    }

    void PacketSender::SendTabComplete(std::string text, bool has_position, Vector<int> block_position)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInTabComplete...");

        int packet_id = 0x14;
        uint8_t* packet = new uint8_t[1024]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WriteString(text, packet, offset);
        PacketEncoder::WriteBoolean(has_position, packet, offset);

        if (has_position)
        {
            PacketEncoder::WritePosition(block_position.GetX(), block_position.GetY(), block_position.GetZ(), packet, offset);
        }

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;
    }

    void PacketSender::SendSettings()
    {
        this->bot->LogDebug(">>> Sending PacketPlayInSettings...");

        int packet_id = 0x15;
        uint8_t* packet = new uint8_t[1024]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WriteString("en_us", packet, offset);
        PacketEncoder::WriteByte(16, packet, offset); // render distance
        PacketEncoder::WriteByte(0, packet, offset); // chat mode
        PacketEncoder::WriteBoolean(true, packet, offset); // chat colors
        PacketEncoder::WriteByte(0x7F, packet, offset); // skin parts

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;
    }

    void PacketSender::SendClientCommand(ClientStatus status)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInClientCommand...");

        int packet_id = 0x16;
        uint8_t* packet = new uint8_t[PacketEncoder::GetVarIntNumBytes(packet_id) + PacketEncoder::GetVarIntNumBytes((int)status)]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WriteVarInt((int)status, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;
    }

    void PacketSender::SendCustomPayload(std::string message)
    {
        this->bot->LogDebug(">>> Sending PacketPlayInCustomPayload...");

        int packet_id = 0x17;
        uint8_t* packet = new uint8_t[1024]{ 0 };
        size_t offset = 0;

        PacketEncoder::WriteVarInt(packet_id, packet, offset);
        PacketEncoder::WriteString(message, packet, offset);

        if (this->bot->GetSocket().send_pack(packet, offset) <= 0)
        {
            this->bot->LogError("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;
    }
}