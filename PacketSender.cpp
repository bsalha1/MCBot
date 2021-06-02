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
        size_t content_length = strlen(content_format) + access_token.length();
        char* content = new char[content_length]{ 0 };
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
        size_t content_length = strlen(content_format) + session.GetAccessToken().length() + session.GetUUID().length() + hash.length();
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
        this->bot->GetLogger().LogDebug(">>> Sending PacketHandshakingIn...");
        this->bot->SetState(State::HANDSHAKE);

        int packet_id = 0x00;
        int protocol_version = 47;
        Packet packet = Packet(1028);
        packet.data = new uint8_t[packet.length]{ 0 };

        PacketEncoder::WriteVarInt(packet_id, packet); // packet id
        PacketEncoder::WriteVarInt(protocol_version, packet);   // protocol version
        PacketEncoder::WriteString(hostname, packet); // hostname
        PacketEncoder::WriteShort(port, packet); // port
        PacketEncoder::WriteVarInt((int)State::LOGIN, packet); // next state

        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;
    }

    void PacketSender::SendLoginStart()
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketLoginInStart...");
        this->bot->SetState(State::LOGIN);

        int packet_id = 0x00;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes(packet_id) + PacketEncoder::GetStringNumBytes(this->bot->GetSession().GetUsername()));
        packet.data = new uint8_t[packet.length]{ 0 };

        PacketEncoder::WriteVarInt(packet_id, packet); // packet id
        PacketEncoder::WriteString(this->bot->GetSession().GetUsername(), packet); // username

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;
    }

    RSA* GetRSAPublicKey(uint8_t* public_key, int public_key_length)
    {
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

        size_t pem_formatted_size = strlen(pem_format) + temp.length() - 2;
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

        return rsa_public_key;
    }

    void PacketSender::SendEncryptionResponse(uint8_t* public_key, int public_key_length, uint8_t* verify_token, int verify_token_length, std::string shared_secret)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketLoginInEncryptionBegin...");

        auto rsa_public_key = GetRSAPublicKey(public_key, public_key_length);

        this->bot->GetLogger().LogDebug("Public Key Info: ");
        BIO* keybio = BIO_new(BIO_s_mem());
        RSA_print(keybio, rsa_public_key, 0);
        char buffer[2048]{ 0 };
        std::string res = "";
        while (BIO_read(keybio, buffer, 2048) > 0)
        {
            this->bot->GetLogger().LogDebug(buffer);
        }
        BIO_free(keybio);

        unsigned char encrypted_shared_secret[256] = { 0 };
        int encrypted_shared_secret_len = RSA_public_encrypt(shared_secret.length(), (unsigned char*)shared_secret.c_str(), encrypted_shared_secret, rsa_public_key, RSA_PKCS1_PADDING);
        this->bot->GetLogger().LogDebug("Encrypted shared secret with public key");

        unsigned char encrypted_verify_token[256] = { 0 };
        int encrypted_verify_token_len = RSA_public_encrypt(verify_token_length, verify_token, encrypted_verify_token, rsa_public_key, RSA_PKCS1_PADDING);
        this->bot->GetLogger().LogDebug("Encrypted verify token with public key");

        int packet_id = 0x01;
        Packet packet = Packet(1028);
        packet.data = new uint8_t[packet.length]{ 0 };

        PacketEncoder::WriteVarInt(packet_id, packet); // packet id

        PacketEncoder::WriteVarInt(encrypted_shared_secret_len, packet); // shared secret length
        PacketEncoder::WriteByteArray(encrypted_shared_secret, encrypted_shared_secret_len, packet); // shared secret

        PacketEncoder::WriteVarInt(encrypted_verify_token_len, packet); // verify token length
        PacketEncoder::WriteByteArray(encrypted_verify_token, encrypted_verify_token_len, packet); // verify token

        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }
        else
        {
            this->bot->InitEncryption((unsigned char*)shared_secret.c_str(), (unsigned char*)shared_secret.c_str());
            this->bot->GetLogger().LogDebug("Sent encryption response - ENCRYPTION ENABLED");
        }

        delete[] packet.data;
    }

    void PacketSender::SendKeepAlive(int id)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInKeepAlive...");

        int packet_id = 0x00;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes({ packet_id, id }));
        packet.data = new uint8_t[packet.length]{ 0 };

        PacketEncoder::WriteVarInt(packet_id, packet); // packet id
        PacketEncoder::WriteVarInt(id, packet);

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;
    }

    void PacketSender::SendChatMessage(std::string message)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInChat...");

        int packet_id = 0x01;
        Packet packet = Packet(message.length() + PacketEncoder::GetVarIntNumBytes({ packet_id, (int)message.length() }));
        packet.data = new uint8_t[packet.length]{ 0 };

        PacketEncoder::WriteVarInt(packet_id, packet);
        PacketEncoder::WriteString(message, packet);

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }
        else
        {
            this->bot->GetLogger().LogDebug("Message: " + message);
        }

        delete[] packet.data;
    }

    void PacketSender::SendUseEntity(int entity_id, UseEntityType action)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInUseEntity...");

        int packet_id = 0x02;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes({ packet_id, entity_id, (int)action }));
        packet.data = new uint8_t[packet.length]{ 0 };

        PacketEncoder::WriteVarInt(packet_id, packet);
        PacketEncoder::WriteVarInt(entity_id, packet);
        PacketEncoder::WriteVarInt((int)action, packet);

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;
    }

    void PacketSender::SendPosition(Vector<double> position, bool on_ground)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInPosition...");

        int packet_id = 0x04;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes(packet_id) + 3 * sizeof(double) + sizeof(bool));
        packet.data = new uint8_t[packet.length]{ 0 };

        PacketEncoder::WriteVarInt(packet_id, packet);
        PacketEncoder::WriteDouble(position.GetX(), packet);
        PacketEncoder::WriteDouble(position.GetY(), packet);
        PacketEncoder::WriteDouble(position.GetZ(), packet);
        PacketEncoder::WriteBoolean(on_ground, packet);

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;

        // Internal updates
        this->bot->UpdatePlayerLocation(position);
    }

    void PacketSender::SendLook(float yaw, float pitch, bool on_ground)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInLook...");

        int packet_id = 0x05;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes(packet_id) + 2 * sizeof(float) + sizeof(bool));
        packet.data = new uint8_t[packet.length]{ 0 };

        PacketEncoder::WriteVarInt(packet_id, packet);
        PacketEncoder::WriteFloat(yaw, packet);
        PacketEncoder::WriteFloat(pitch, packet);
        PacketEncoder::WriteBoolean(on_ground, packet);

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;

        // Internal updates
        this->bot->UpdatePlayerRotation(yaw, pitch);
    }

    void PacketSender::SendPositionLook(Vector<double> position, float yaw, float pitch, bool on_ground)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInPositionLook...");

        int packet_id = 0x06;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes(packet_id) + 3 * sizeof(double) + 2 * sizeof(float) + 1);
        packet.data = new uint8_t[packet.length]{ 0 };

        PacketEncoder::WriteVarInt(packet_id, packet);
        PacketEncoder::WriteDouble(position.GetX(), packet);
        PacketEncoder::WriteDouble(position.GetY(), packet);
        PacketEncoder::WriteDouble(position.GetZ(), packet);
        PacketEncoder::WriteFloat(yaw, packet);
        PacketEncoder::WriteFloat(pitch, packet);
        PacketEncoder::WriteBoolean(on_ground, packet);

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;

        // Internal updates
        this->bot->UpdatePlayerRotation(yaw, pitch);
        this->bot->UpdatePlayerLocation(position);
    }

    void PacketSender::SendBlockDig(DigStatus status, Vector<int> location, BlockFace face)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInBlockDig...");

        int packet_id = 0x07;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes(packet_id) + 10);
        packet.data = new uint8_t[packet.length]{ 0 };

        PacketEncoder::WriteVarInt(packet_id, packet);
        PacketEncoder::WriteByte((uint8_t)status, packet);
        PacketEncoder::WritePosition(location.GetX(), location.GetY(), location.GetZ(), packet);
        PacketEncoder::WriteByte((uint8_t)face, packet);

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;
    }

    void PacketSender::SendBlockPlace(Vector<int> location, BlockFace face, Slot held_item, Vector<uint8_t> cursor_position)
    {
        /*this->bot->log_debug(">>> Sending PacketPlayInBlockPlace...");

        int packet_id = 0x08;
        uint8_t* packet = new uint8_t[PacketEncoder::get_var_int_size(packet_id) + 10]{ 0 };
        size_t offset = 0;

        PacketEncoder::write_var_int(packet_id, packet);
        PacketEncoder::write_position(location.get_x(), location.get_y(), location.get_z(), packet);
        PacketEncoder::write_byte((uint8_t)face, packet);
        PacketEncoder::write_short()

        if (this->bot->get_socket().send_pack(packet) <= 0)
        {
            this->bot->log_error("Failed to send packet");
            print_winsock_error();
        }

        delete[] packet;*/
    }

    void PacketSender::SendHeldItemSlot(short slot)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInHeldItemSlot...");

        int packet_id = 0x09;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes(packet_id) + sizeof(short));
        packet.data = new uint8_t[packet.length]{ 0 };

        PacketEncoder::WriteVarInt(packet_id, packet);
        PacketEncoder::WriteShort(slot, packet);

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;
    }

    void PacketSender::SendArmAnimation()
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInArmAnimation...");

        int packet_id = 0x0A;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes(packet_id));
        packet.data = new uint8_t[packet.length]{ 0 };

        PacketEncoder::WriteVarInt(packet_id, packet);

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;
    }

    void PacketSender::SendEntityAction(int player_entity_id, EntityAction action, int param)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInEntityAction...");

        int packet_id = 0x0B;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes({ packet_id, player_entity_id, (int)action, param }));
        packet.data = new uint8_t[packet.length];

        PacketEncoder::WriteVarInt(packet_id, packet);
        PacketEncoder::WriteVarInt(player_entity_id, packet);
        PacketEncoder::WriteVarInt((int)action, packet);
        PacketEncoder::WriteVarInt(param, packet);

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;
    }

    void PacketSender::SendSetCreativeSlot(short slot, Slot item)
    {
        //this->bot->log_debug(">>> Sending PacketPlayInSetCreativeSlot...");

        //int packet_id = 0x10;
        //Packet packet = Packet(1024);
        //size_t offset = 0;

        //PacketEncoder::write_var_int(packet_id, packet);
        //PacketEncoder::write_short(slot, packet);

        //if (this->bot->get_socket().send_pack(packet) <= 0)
        //{
        //    this->bot->log_error("Failed to send packet");
        //    print_winsock_error();
        //}

        //delete[] packet;
    }

    void PacketSender::SendEnchantItem(uint8_t window_id, uint8_t enchantment)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInEnchantItem..");

        int packet_id = 0x11;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes(packet_id) + 2 * sizeof(uint8_t));
        packet.data = new uint8_t[packet.length];

        PacketEncoder::WriteVarInt(packet_id, packet);
        PacketEncoder::WriteByte(window_id, packet);
        PacketEncoder::WriteByte(enchantment, packet);

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;
    }

    void PacketSender::SendUpdateSign(Vector<int> location, std::string line1, std::string line2, std::string line3, std::string line4)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInUpdateSign...");

        int packet_id = 0x12;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes(packet_id) + sizeof(long) + PacketEncoder::GetStringNumBytes({line1, line2, line3, line4}));
        packet.data = new uint8_t[packet.length];

        PacketEncoder::WriteVarInt(packet_id, packet);
        PacketEncoder::WritePosition(location.GetX(), location.GetY(), location.GetZ(), packet);
        PacketEncoder::WriteString(line1, packet);
        PacketEncoder::WriteString(line2, packet);
        PacketEncoder::WriteString(line3, packet);
        PacketEncoder::WriteString(line4, packet);

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;
    }

    void PacketSender::SendAbilities(uint8_t flags, float flying_speed, float walking_speed)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInAbilities...");

        int packet_id = 0x13;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes(packet_id) + sizeof(uint8_t) + 2 * sizeof(float));
        packet.data = new uint8_t[packet.length];

        PacketEncoder::WriteVarInt(packet_id, packet);
        PacketEncoder::WriteByte(flags, packet);
        PacketEncoder::WriteFloat(flying_speed, packet);
        PacketEncoder::WriteFloat(walking_speed, packet);

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;
    }

    void PacketSender::SendTabComplete(std::string text, bool has_position, Vector<int> block_position)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInTabComplete...");

        int packet_id = 0x14;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes(packet_id) + PacketEncoder::GetStringNumBytes(text) + sizeof(bool) + (has_position ? sizeof(long) : 0));
        packet.data = new uint8_t[packet.length];

        PacketEncoder::WriteVarInt(packet_id, packet);
        PacketEncoder::WriteString(text, packet);
        PacketEncoder::WriteBoolean(has_position, packet);

        if (has_position)
        {
            PacketEncoder::WritePosition(block_position.GetX(), block_position.GetY(), block_position.GetZ(), packet);
        }

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;
    }

    void PacketSender::SendSettings()
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInSettings...");

        int packet_id = 0x15;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes(packet_id) + PacketEncoder::GetStringNumBytes("en_us") + 3 * sizeof(uint8_t) + 1 * sizeof(bool));
        packet.data = new uint8_t[packet.length];

        PacketEncoder::WriteVarInt(packet_id, packet);
        PacketEncoder::WriteString("en_us", packet);
        PacketEncoder::WriteByte(16, packet); // render distance
        PacketEncoder::WriteByte(0, packet); // chat mode
        PacketEncoder::WriteBoolean(true, packet); // chat colors
        PacketEncoder::WriteByte(0x7F, packet); // skin parts

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;
    }

    void PacketSender::SendClientCommand(ClientStatus status)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInClientCommand...");

        int packet_id = 0x16;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes(packet_id) + PacketEncoder::GetVarIntNumBytes((int)status));
        packet.data = new uint8_t[packet.length];

        PacketEncoder::WriteVarInt(packet_id, packet);
        PacketEncoder::WriteVarInt((int)status, packet);

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;
    }

    void PacketSender::SendCustomPayload(std::string message)
    {
        this->bot->GetLogger().LogDebug(">>> Sending PacketPlayInCustomPayload...");

        int packet_id = 0x17;
        Packet packet = Packet(PacketEncoder::GetVarIntNumBytes(packet_id) + PacketEncoder::GetStringNumBytes(message));
        packet.data = new uint8_t[packet.length];

        PacketEncoder::WriteVarInt(packet_id, packet);
        PacketEncoder::WriteString(message, packet);

        ASSERT_TRUE(packet.length == packet.offset, "Unexpected packet size");
        if (ASSERT_TRUE(this->bot->SendPacket(packet) > 0, "Failed to send packet"))
        {
            print_winsock_error();
        }

        delete[] packet.data;
    }
}