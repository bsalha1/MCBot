#include "MCBot.h"
#include "PacketSender.h"

static void print_winsock_error()
{
    wchar_t* s = NULL;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&s, 0, NULL);
    printf("%S\n", s);
}

mcbot::PacketSender::PacketSender(MCBot* bot)
{
    this->bot = bot;
}

mcbot::PacketSender::PacketSender()
{
    this->bot = NULL;
}

int mcbot::PacketSender::login_mojang(std::string email, std::string password)
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

    this->bot->set_session(MojangSession(access_token, username, uuid));

    return 0;
}

int mcbot::PacketSender::verify_access_token()
{
    // Payload //
    const char* content_format =
        "{"
        "\"accessToken\" : \"%s\""
        "}\r\n";

    std::string access_token = this->bot->get_session().get_access_token();
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

int mcbot::PacketSender::send_session(std::string server_id, std::string shared_secret, uint8_t* public_key, int public_key_length)
{
    mcbot::daft_hash_impl hasher;
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

    MojangSession session = this->bot->get_session();
    int content_length = strlen(content_format) + session.get_access_token().length() + session.get_uuid().length() + hash.length();
    char* content = new char[content_length] {0};
    sprintf_s(content, content_length, content_format, session.get_access_token().c_str(), session.get_uuid().c_str(), hash.c_str());

    // Send Payload //
    httplib::Client cli("https://sessionserver.mojang.com");
    auto response = cli.Post("/session/minecraft/join", content, "application/json");

    delete[] content;

    // If response is 204 then session is valid
    return response->status == 204 ? 0 : -1;
}

void mcbot::PacketSender::send_handshake(char* hostname, unsigned short port)
{
    this->bot->log_debug(">>> Sending PacketHandshakingIn...");
    this->bot->set_state(mcbot::State::HANDSHAKE);

    uint8_t packet[1028];
    size_t offset = 0;

    PacketEncoder::write_var_int(0x00, packet, offset); // packet id
    PacketEncoder::write_var_int(47, packet, offset);   // protocol version
    PacketEncoder::write_string(hostname, packet, offset); // hostname
    PacketEncoder::write_short(port, packet, offset); // port
    PacketEncoder::write_var_int((int)mcbot::State::LOGIN, packet, offset); // next state

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }
}

void mcbot::PacketSender::send_login_start()
{
    this->bot->log_debug(">>> Sending PacketLoginInStart...");
    this->bot->set_state(State::LOGIN);

    uint8_t packet[1028];
    size_t offset = 0;

    PacketEncoder::write_var_int(0x00, packet, offset); // packet id
    PacketEncoder::write_string((char*)this->bot->get_session().get_username().c_str(), packet, offset); // username

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }
}

void mcbot::PacketSender::send_encryption_response(uint8_t* public_key, int public_key_length, uint8_t* verify_token, int verify_token_length, std::string shared_secret)
{
    this->bot->log_debug(">>> Sending PacketLoginInEncryptionBegin...");
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

    this->bot->log_debug("Public Key Info: ");
    BIO* keybio = BIO_new(BIO_s_mem());
    RSA_print(keybio, rsa_public_key, 0);
    char buffer[2048] = { 0 };
    std::string res = "";
    while (BIO_read(keybio, buffer, 2048) > 0)
    {
        this->bot->log_debug(buffer);
    }
    BIO_free(keybio);

    unsigned char encrypted_shared_secret[256] = { 0 };
    int encrypted_shared_secret_len = RSA_public_encrypt(shared_secret.length(), (unsigned char*)shared_secret.c_str(), encrypted_shared_secret, rsa_public_key, RSA_PKCS1_PADDING);
    this->bot->log_debug("Encrypted shared secret with public key");

    unsigned char encrypted_verify_token[256] = { 0 };
    int encrypted_verify_token_len = RSA_public_encrypt(verify_token_length, verify_token, encrypted_verify_token, rsa_public_key, RSA_PKCS1_PADDING);
    this->bot->log_debug("Encrypted verify token with public key");

    uint8_t packet[1028];
    size_t offset = 0;

    PacketEncoder::write_var_int(0x01, packet, offset); // packet id

    PacketEncoder::write_var_int(encrypted_shared_secret_len, packet, offset); // shared secret length
    PacketEncoder::write_byte_array(encrypted_shared_secret, encrypted_shared_secret_len, packet, offset); // shared secret

    PacketEncoder::write_var_int(encrypted_verify_token_len, packet, offset); // verify token length
    PacketEncoder::write_byte_array(encrypted_verify_token, encrypted_verify_token_len, packet, offset); // verify token

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }
    else
    {
        this->bot->get_socket().initialize_encryption((unsigned char*)shared_secret.c_str(), (unsigned char*)shared_secret.c_str());
        this->bot->log_debug("Sent encryption response - ENCRYPTION ENABLED");
    }
}

void mcbot::PacketSender::send_keep_alive(int id)
{
    this->bot->log_debug(">>> Sending PacketPlayInKeepAlive...");

    int packet_id = 0x00;
    uint8_t* packet = new uint8_t[PacketEncoder::get_var_int_size({ packet_id, id })];
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset); // packet id
    PacketEncoder::write_var_int(id, packet, offset);

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}

void mcbot::PacketSender::send_chat_message(std::string message)
{
    this->bot->log_debug(">>> Sending PacketPlayInChat...");

    int packet_id = 0x01;
    uint8_t* packet = new uint8_t[message.length() + PacketEncoder::get_var_int_size({ packet_id, (int)message.length()}) + 1]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_string(message, packet, offset);

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }
    else
    {
        this->bot->log_debug("Message: " + message);
    }

    delete[] packet;
}

void mcbot::PacketSender::send_use_entity(int entity_id, mcbot::UseEntityType action)
{
    this->bot->log_debug(">>> Sending PacketPlayInUseEntity...");

    int packet_id = 0x02;
    uint8_t* packet = new uint8_t[PacketEncoder::get_var_int_size({packet_id, entity_id, (int)action})]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_var_int(entity_id, packet, offset);
    PacketEncoder::write_var_int((int)action, packet, offset);

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}

void mcbot::PacketSender::send_position(Vector<double> position, bool on_ground)
{
    this->bot->log_debug(">>> Sending PacketPlayInPosition...");


    int packet_id = 0x04;
    uint8_t* packet = new uint8_t[PacketEncoder::get_var_int_size(packet_id) + 3 * sizeof(double) + 1]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_double(position.get_x(), packet, offset);
    PacketEncoder::write_double(position.get_y(), packet, offset);
    PacketEncoder::write_double(position.get_z(), packet, offset);
    PacketEncoder::write_boolean(on_ground, packet, offset);

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;

    // Internal updates
    this->bot->get_player().update_location(position);
}

void mcbot::PacketSender::send_look(float yaw, float pitch, bool on_ground)
{
    this->bot->log_debug(">>> Sending PacketPlayInLook...");

    int packet_id = 0x05;
    uint8_t* packet = new uint8_t[PacketEncoder::get_var_int_size(packet_id) + 2 * sizeof(float) + 1]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_float(yaw, packet, offset);
    PacketEncoder::write_float(pitch, packet, offset);
    PacketEncoder::write_boolean(on_ground, packet, offset);

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;

    // Internal updates
    this->bot->get_player().update_rotation(yaw, pitch);
}

void mcbot::PacketSender::send_position_look(Vector<double> position, float yaw, float pitch, bool on_ground)
{
    this->bot->log_debug(">>> Sending PacketPlayInPositionLook...");

    int packet_id = 0x06;
    uint8_t* packet = new uint8_t[PacketEncoder::get_var_int_size(packet_id) + 3 * sizeof(double) + 2  * sizeof(float) + 1]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_double(position.get_x(), packet, offset);
    PacketEncoder::write_double(position.get_y(), packet, offset);
    PacketEncoder::write_double(position.get_z(), packet, offset);
    PacketEncoder::write_float(yaw, packet, offset);
    PacketEncoder::write_float(pitch, packet, offset);
    PacketEncoder::write_boolean(on_ground, packet, offset);

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;

    // Internal updates
    this->bot->get_player().update_location(position);
    this->bot->get_player().update_rotation(yaw, pitch);
}

void mcbot::PacketSender::send_block_dig(DigStatus status, mcbot::Vector<int> location, BlockFace face)
{
    this->bot->log_debug(">>> Sending PacketPlayInBlockDig...");

    int packet_id = 0x07;
    uint8_t* packet = new uint8_t[PacketEncoder::get_var_int_size(packet_id) + 10]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_byte((uint8_t)status, packet, offset);
    PacketEncoder::write_position(location.get_x(), location.get_y(), location.get_z(), packet, offset);
    PacketEncoder::write_byte((uint8_t)face, packet, offset);

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}

void mcbot::PacketSender::send_block_place(mcbot::Vector<int> location, BlockFace face, Slot held_item, mcbot::Vector<uint8_t> cursor_position)
{
    this->bot->log_debug(">>> Sending PacketPlayInBlockPlace...");

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

    delete[] packet;
}

void mcbot::PacketSender::send_held_item_slot(short slot)
{
    this->bot->log_debug(">>> Sending PacketPlayInHeldItemSlot...");

    int packet_id = 0x09;
    uint8_t* packet = new uint8_t[PacketEncoder::get_var_int_size(packet_id) + sizeof(short)]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_short(slot, packet, offset);

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}

void mcbot::PacketSender::send_arm_animation()
{
    this->bot->log_debug(">>> Sending PacketPlayInArmAnimation...");

    int packet_id = 0x0A;
    uint8_t* packet = new uint8_t[PacketEncoder::get_var_int_size(packet_id)]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}


void mcbot::PacketSender::send_entity_action(int player_id, EntityAction action, int param)
{
    this->bot->log_debug(">>> Sending PacketPlayInEntityAction...");

    int packet_id = 0x0B;
    uint8_t* packet = new uint8_t[PacketEncoder::get_var_int_size({packet_id, player_id, (int)action, param})]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_var_int(player_id, packet, offset);
    PacketEncoder::write_var_int((int)action, packet, offset);
    PacketEncoder::write_var_int(param, packet, offset);

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}

void mcbot::PacketSender::send_set_creative_slot(short slot, Slot item)
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

void mcbot::PacketSender::send_enchant_item(uint8_t window_id, uint8_t enchantment)
{
    this->bot->log_debug(">>> Sending PacketPlayInEnchantItem..");

    int packet_id = 0x11;
    uint8_t* packet = new uint8_t[1024]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_byte(window_id, packet, offset);
    PacketEncoder::write_byte(enchantment, packet, offset);

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}

void mcbot::PacketSender::send_update_sign(mcbot::Vector<int> location, std::string line1, std::string line2, std::string line3, std::string line4)
{
    this->bot->log_debug(">>> Sending PacketPlayInUpdateSign...");

    int packet_id = 0x12;
    uint8_t* packet = new uint8_t[1024]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_position(location.get_x(), location.get_y(), location.get_z(), packet, offset);
    PacketEncoder::write_string(line1, packet, offset);
    PacketEncoder::write_string(line2, packet, offset);
    PacketEncoder::write_string(line3, packet, offset);
    PacketEncoder::write_string(line4, packet, offset);

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}

void mcbot::PacketSender::send_abilities(uint8_t flags, float flying_speed, float walking_speed)
{
    this->bot->log_debug(">>> Sending PacketPlayInAbilities...");

    int packet_id = 0x13;
    uint8_t* packet = new uint8_t[PacketEncoder::get_var_int_size(packet_id) + sizeof(uint8_t) + 2 * sizeof(float)]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_byte(flags, packet, offset);
    PacketEncoder::write_float(flying_speed, packet, offset);
    PacketEncoder::write_float(walking_speed, packet, offset);

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}

void mcbot::PacketSender::send_tab_complete(std::string text, bool has_position, mcbot::Vector<int> block_position)
{
    this->bot->log_debug(">>> Sending PacketPlayInTabComplete...");

    int packet_id = 0x14;
    uint8_t* packet = new uint8_t[1024]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_string(text, packet, offset);
    PacketEncoder::write_boolean(has_position, packet, offset);

    if (has_position)
    {
        PacketEncoder::write_position(block_position.get_x(), block_position.get_y(), block_position.get_z(), packet, offset);
    }

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}

void mcbot::PacketSender::send_settings()
{
    this->bot->log_debug(">>> Sending PacketPlayInSettings...");

    int packet_id = 0x15;
    uint8_t* packet = new uint8_t[1024]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_string("en_us", packet, offset);
    PacketEncoder::write_byte(16, packet, offset); // render distance
    PacketEncoder::write_byte(0, packet, offset); // chat mode
    PacketEncoder::write_boolean(true, packet, offset); // chat colors
    PacketEncoder::write_byte(0x7F, packet, offset); // skin parts

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}

void mcbot::PacketSender::send_client_command(ClientStatus status)
{
    this->bot->log_debug(">>> Sending PacketPlayInClientCommand...");

    int packet_id = 0x16;
    uint8_t* packet = new uint8_t[PacketEncoder::get_var_int_size(packet_id) + PacketEncoder::get_var_int_size((int)status)]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_var_int((int)status, packet, offset);

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}

void mcbot::PacketSender::send_custom_payload(std::string message)
{
    this->bot->log_debug(">>> Sending PacketPlayInCustomPayload...");

    int packet_id = 0x17;
    uint8_t* packet = new uint8_t[1024]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_string(message, packet, offset);

    if (this->bot->get_socket().send_pack(packet, offset) <= 0)
    {
        this->bot->log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}