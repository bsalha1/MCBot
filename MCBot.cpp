#include "MCBot.h"

#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>

#include "PacketDecoder.h"
#include "PacketEncoder.h"

#include "StringUtils.h"
#include "DaftHash.h"
#include "base64.h"

#include "zlib.h"


static void print_winsock_error()
{
    wchar_t* s = NULL;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&s, 0, NULL);
    printf("%S\n", s);
}

static std::string get_random_hex_bytes(std::size_t num_bytes)
{
    std::string out;

    for (std::size_t i = 0; i < num_bytes; i++)
    {
        int val = rand() % 0xF;

        char hex_string[33];
        _itoa_s(val, hex_string, 16);
        out.append(hex_string);
    }
    return out;
}

int mcbot::MCBot::read_next_var_int()
{
    int num_read = 0;
    int result = 0;
    char read;
    int i = 0;
    do {

        uint8_t packet[2] = { 0 };
        size_t bytes_read = this->sock.recv_packet(packet, 1);
        if (bytes_read < 0)
        {
            std::cout << "Failed to receive packet" << std::endl;
            return -1;
        }
        read = packet[0];


        int value = (read & 0b01111111);
        result |= (value << (7 * num_read));

        num_read++;
        if (num_read > 5)
        {
            fprintf(stderr, "VarInt out of bounds");
            return -1;
        }
    } while ((read & 0b10000000) != 0);
    return result;
}

int mcbot::MCBot::read_next_packet(int length, uint8_t* packet, int decompressed_length)
{
    int bytes_read = this->sock.recv_packet(packet, length, decompressed_length);
    if (bytes_read < 0)
    {
        log_error("Failed to receive packet");
        packet = NULL;
        return bytes_read;
    }
    log_debug("Received Packet: " + std::to_string(bytes_read) + "bytes");

    return bytes_read;
}

void mcbot::MCBot::update_player_info(mcbot::PlayerInfoAction action, int length, uint8_t* packet, size_t& offset)
{
    for (int i = 0; i < length; i++)
    {
        mcbot::UUID uuid = PacketDecoder::read_uuid(packet, offset);

        log_debug("\tPlayer Update (" + uuid.to_string() + "): " + mcbot::to_string(action));

        mcbot::EntityPlayer player;

        switch (action)
        {
        case mcbot::PlayerInfoAction::ADD_PLAYER:
        {
            std::string name = PacketDecoder::read_string(packet, offset);
            int properties_length = PacketDecoder::read_var_int(packet, offset);
            std::list<mcbot::PlayerProperty> properties = PacketDecoder::read_property_array(properties_length, packet, offset);
            mcbot::Gamemode gamemode = (mcbot::Gamemode) PacketDecoder::read_var_int(packet, offset);
            int ping = PacketDecoder::read_var_int(packet, offset);
            bool has_display_name = PacketDecoder::read_boolean(packet, offset);
            std::string display_name = has_display_name ? PacketDecoder::read_string(packet, offset) : name;

            player = mcbot::EntityPlayer(-1, uuid, name, properties, gamemode, ping, display_name);

            if (name == this->username)
            {
                this->player.set_properties(properties);
                this->player.set_gamemode(gamemode);
                this->player.set_ping(ping);
                this->player.set_display_name(display_name);
            }
            
            this->uuid_to_player.insert(std::pair<mcbot::UUID, mcbot::EntityPlayer>(uuid, player));

            log_debug("\t\tName: " + name + '\n'
                + "\t\tGamemode: " + mcbot::to_string(gamemode) + '\n'
                + "\t\tPing: " + std::to_string(ping));
            break; 
        }

        case mcbot::PlayerInfoAction::UPDATE_GAMEMODE:
        {
            try
            {
                player = this->uuid_to_player.at(uuid);
            }
            catch (...)
            {
                log_error("Failed to find player of UUID " + uuid.to_string());
                return;
            }
            mcbot::Gamemode gamemode = (mcbot::Gamemode) PacketDecoder::read_var_int(packet, offset);
            player.set_gamemode(gamemode);
            break;
        }

        case mcbot::PlayerInfoAction::UPDATE_LATENCY:
        {
            try
            {
                player = this->uuid_to_player.at(uuid);
            }
            catch (...)
            {
                log_error("Failed to find player of UUID " + uuid.to_string());
                return;
            }
            int ping = PacketDecoder::read_var_int(packet, offset);

            player.set_ping(ping);
            break;
        }

        case mcbot::PlayerInfoAction::UPDATE_DISPLAY_NAME:
        {
            try
            {
                player = this->uuid_to_player.at(uuid);
            }
            catch (...)
            {
                log_error("Failed to find player of UUID " + uuid.to_string());
                return;
            }

            bool has_display_name = PacketDecoder::read_boolean(packet, offset);
            std::string display_name = has_display_name ? PacketDecoder::read_string(packet, offset) : "";
            if (has_display_name)
            {
                player.set_display_name(display_name);
            }
            
            break;
        }

        case mcbot::PlayerInfoAction::REMOVE_PLAYER:
        {
            this->uuid_to_player.erase(uuid);
            break;
        }
        default:
            break;
        }
    }
}

mcbot::MCBot::MCBot(std::string email, std::string password)
{
    this->email = email;
    this->password = password;

    this->debug = false;
    this->connected = false;
    this->ready = false;
    this->encryption_enabled = false;
    this->compression_enabled = false;
    this->uuid_to_player = std::map<mcbot::UUID, mcbot::EntityPlayer>();
    this->world_border = mcbot::WorldBorder();

    // Start WinSock DLL //
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout << "Failed to start up WinSock DLL" << std::endl;
        print_winsock_error();
        return;
    }
}

mcbot::MCBot::~MCBot()
{
    delete[] this->verify_token;
    delete[] this->public_key;
}

void mcbot::MCBot::move_to(mcbot::Vector<double> destination, double speed, bool ignore_ground)
{
    while (this->player.get_location().distance(destination) >= 0)
    {
        Vector<double> current_location = this->player.get_location();
        Vector<double> direction = destination - current_location;

        direction.scale(1 / direction.magnitude());
        direction.scale(speed);

        // If we are close enough, just move us to the location instantly
        if (this->player.get_location().distance(destination) <= 1.0)
        {
            this->send_position(destination, true);
            return;
        }

        try
        {
            this->move(direction);
        }
        catch (const CollisionException & e)
        {
            std::cerr << e.what() << std::endl;
            this->send_position(direction + mcbot::Vector<double>(0, 1, 0), true);
        }

        if (!ignore_ground && !this->on_ground())
        {
            this->move_to_ground(0.10);
        }

        Sleep(1000 / 20);
    }
}

void mcbot::MCBot::move_to(double x, double z, int speed)
{
    mcbot::Vector<double> dest = mcbot::Vector<double>(x, 0, z);
    mcbot::Vector<double> init = this->player.get_location();

    auto diff = dest - init;

    double dx = abs(diff.get_x()) / diff.get_x();
    double dz = abs(diff.get_z()) / diff.get_z();
    
    for (double x = init.get_x(); x != dest.get_x(); x += dx)
    {
        Vector<double> current_location = this->player.get_location();

        try
        {
            this->move(dx, 0);
        }
        catch (const CollisionException & e)
        {
            std::cerr << e.what() << std::endl;
            this->send_position(this->player.get_location() + mcbot::Vector<double>(0, 1, 0), false);
            Sleep(1000 / 20 * speed);
            this->send_position(this->player.get_location() + mcbot::Vector<double>(abs(dx)/dx, 0, 0) , true);
            Sleep(1000 / 20 * speed);
            continue;
        }

        if (!this->on_ground())
        {
            Sleep(1000 / 20 * speed);
            this->move_to_ground(0.10);
        }

        Sleep(1000 / 20 * speed);
    }


    for (double z = init.get_z(); z != dest.get_z(); z += dz)
    {
        Vector<double> current_location = this->player.get_location();

        try
        {
            this->move(0, dz);
        }
        catch (const CollisionException & e)
        {
            std::cerr << e.what() << std::endl;
            this->send_position(this->player.get_location() + mcbot::Vector<double>(0, 1, 0), false);
            Sleep(1000 / 20);
            this->send_position(this->player.get_location() + mcbot::Vector<double>(0, 0, abs(dz)/dz), true);
            continue;
        }

        if (!this->on_ground())
        {
            Sleep(1000 / 20);
            this->move_to_ground(0.10);
        }

        Sleep(1000 / 20);
    }
}

void mcbot::MCBot::load_chunk(Chunk chunk)
{
    this->chunks.insert(std::pair<std::pair<int, int>, Chunk>(std::pair<int, int>(chunk.get_x(), chunk.get_z()), chunk));
}

mcbot::Chunk mcbot::MCBot::get_chunk(int x, int z)
{
    return this->chunks.at(std::pair<int, int>(x, z));
}

mcbot::Chunk mcbot::MCBot::get_chunk(mcbot::Vector<double> location)
{
    int x = ((int)floor(location.get_x())) >> 4;
    int z = ((int)floor(location.get_z())) >> 4;
    return this->chunks.at(std::pair<int, int>(x, z));
}

mcbot::Vector<double> mcbot::MCBot::get_ground_location(mcbot::Vector<double> location)
{
    Chunk chunk = get_chunk(location);
    location.set_y(floor(location.get_y()));

    while (location.get_y() >= 0)
    {
        Block block = Block(chunk.get_block_id(location));
        if (!block.is_weak())
        {
            break;
        }
        location = location - mcbot::Vector<double>(0, 1, 0);
    }

    return location;
}

void mcbot::MCBot::move_to_ground(double speed)
{
    if (this->on_ground())
    {
        return;
    }
    std::cout << "Moving to ground" << std::endl;
    mcbot::Vector<double> dest = this->get_ground_location(this->player.get_location()) + mcbot::Vector<double>(0, 1, 0);
    this->move_to(dest, speed, true);
}

bool mcbot::MCBot::on_ground()
{
    mcbot::Vector<double> player_location = this->player.get_location();
    mcbot::Vector<double> ground_location = this->get_ground_location(player_location) + mcbot::Vector<double>(0, 1, 0);

    return player_location.get_y() == ground_location.get_y();
}

void mcbot::MCBot::move(mcbot::Vector<double> diff)
{
    mcbot::Vector<double> init = this->player.get_location();
    mcbot::Vector<double> dest = init + diff;

    Chunk dest_chunk = get_chunk(dest);
    Chunk init_chunk = get_chunk(init);

    int dest_block_id = dest_chunk.get_block_id(dest);

    // Collision detection
    if (dest_block_id != 0 && dest_block_id != 31)
    {
        throw CollisionException(dest_block_id, dest);
    }

    this->send_position(dest, true);
}

void mcbot::MCBot::move(double dx, double dz) 
{
    mcbot::Vector<double> dr = mcbot::Vector<double>(dx, 0, dz);
    mcbot::Vector<double> init = this->player.get_location();
    mcbot::Vector<double> dest = init + dr;

    Chunk dest_chunk = get_chunk(dest);
    Chunk init_chunk = get_chunk(init);
    Block dest_block = Block(dest_chunk.get_block_id(dest));


    // Collision detection
    if (!dest_block.is_weak())
    {
        std::cout << dest_block.get_id() << std::endl;
        throw CollisionException(dest_block, dest);
    }

    this->send_position(dest, true);
}

void mcbot::MCBot::log_debug(std::string message)
{
    if (!this->debug)
    {
        return;
    }

    std::cout << "\33[2K\r" << "[DEBUG] " << message << std::endl;
    std::cout << "> ";
}

void mcbot::MCBot::log_error(std::string message)
{
    std::cout << "\33[2K\r" << "[ERROR] " << message << std::endl;
    std::cout << "> ";
}

void mcbot::MCBot::log_info(std::string message)
{
    std::cout << "\33[2K\r" << "[INFO] " << message << std::endl;
    std::cout << "> ";
}

void mcbot::MCBot::log_chat(std::string message)
{
    std::cout << "\33[2K\r" << "[CHAT] " << message << std::endl;
    std::cout << "> ";
}

void mcbot::MCBot::set_debug(bool debug)
{
    this->debug = debug;
}

int mcbot::MCBot::login_mojang()
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
    sprintf_s(content, content_format, this->email.c_str(), this->password.c_str());

    // Send Payload //
    httplib::Client cli("https://authserver.mojang.com");
    auto response = cli.Post("/authenticate", content, "application/json");

    // Read Response //
    auto response_json = nlohmann::json::parse(response->body);
    this->access_token = response_json["accessToken"];
    
    auto selected_profile = response_json["selectedProfile"];
    this->username = selected_profile["name"];
    this->uuid = selected_profile["id"];

    std::cout << "Logged into Mojang account:" << std::endl
        << "\tAccount resolved to " << this->username << std::endl;

    return 0;
}

int mcbot::MCBot::verify_access_token()
{
    // Payload //
    const char *content_format = 
        "{"
            "\"accessToken\" : \"%s\""
        "}\r\n";

    int content_length = strlen(content_format) + this->access_token.length();
    char* content = new char[content_length] {0};
    sprintf_s(content, content_length, content_format, this->access_token.c_str());

    // Send Payload //
    httplib::Client cli("https://authserver.mojang.com");
    auto response = cli.Post("/validate", content, "application/json");

    delete[] content;

    // If response is 204 then the access token is valid
    return response->status == 204 ? 0 : -1;
}

int mcbot::MCBot::send_session()
{
    this->shared_secret = get_random_hex_bytes(16);

    mcbot::daft_hash_impl hasher;
    hasher.update((void*)this->server_id.c_str(), this->server_id.length());
    hasher.update((void*)this->shared_secret.c_str(), this->shared_secret.length());
    hasher.update((void*)this->public_key, public_key_length);

    std::string hash = hasher.finalise();
    

    // Payload //
    const char* content_format =
        "{"
            "\"accessToken\" : \"%s\","
            "\"selectedProfile\": \"%s\","
            "\"serverId\": \"%s\""
        "}\r\n";

    int content_length = strlen(content_format) + this->access_token.length() + this->uuid.length() + hash.length();
    char* content = new char[content_length] {0};
    sprintf_s(content, content_length, content_format, this->access_token.c_str(), this->uuid.c_str(), hash.c_str());

    // Send Payload //
    httplib::Client cli("https://sessionserver.mojang.com");
    auto response = cli.Post("/session/minecraft/join", content, "application/json");

    delete[] content;

    // If response is 204 then session is valid
    return response->status == 204 ? 0 : -1;
}

int mcbot::MCBot::connect_server(char* hostname, char* port)
{
    std::string hostname_s = hostname;

    // Resolve Host //
    struct addrinfo hints;
    struct addrinfo* result;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if (getaddrinfo(hostname, port, &hints, &result) < 0 || result == NULL)
    {
        log_error("Failed to resolve " + hostname_s);
        print_winsock_error();
        WSACleanup();
        return -1;
    }

    char* address_string = inet_ntoa(((struct sockaddr_in*) result->ai_addr)->sin_addr);
    std::string address_string_s = address_string;

    log_info("Resolved " + hostname_s + " to " + address_string_s);

    // Connect //
    this->sock = mcbot::Socket(socket(result->ai_family, result->ai_socktype, result->ai_protocol));
    log_info("Connecting to " + address_string_s + ":" + port);
    if (this->sock.connect_socket(result) < 0)
    {
        print_winsock_error();
        WSACleanup();
        return -1;
    }

    this->connected = true;
    log_info("Connected to " + address_string_s + ":" + port);

    return 0;
}

void mcbot::MCBot::send_handshake(char* hostname, unsigned short port)
{
    log_debug(">>> Sending PacketHandshakingIn...");
    this->state = mcbot::State::HANDSHAKE;

    uint8_t packet[1028];
    size_t offset = 0;

    PacketEncoder::write_var_int(0x00, packet, offset); // packet id
    PacketEncoder::write_var_int(47, packet, offset);   // protocol version
    PacketEncoder::write_string(hostname, packet, offset); // hostname
    PacketEncoder::write_short(port, packet, offset); // port
    PacketEncoder::write_var_int((int)mcbot::State::LOGIN, packet, offset); // next state

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        log_error("Failed to send packet");
        print_winsock_error();
    }
}

void mcbot::MCBot::send_login_start()
{
    log_debug(">>> Sending PacketLoginInStart...");
    this->state = State::LOGIN;

    uint8_t packet[1028];
    size_t offset = 0;

    PacketEncoder::write_var_int(0x00, packet, offset); // packet id
    PacketEncoder::write_string((char*) this->username.c_str(), packet, offset); // username

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        log_error("Failed to send packet");
        print_winsock_error();
    }
}

void mcbot::MCBot::send_encryption_response()
{
    log_debug(">>> Sending PacketLoginInEncryptionBegin...");
    // Convert DER to PEM //
    char pem_format[] = 
        "-----BEGIN PUBLIC KEY-----"
        "%s\n"
        "-----END PUBLIC KEY-----\n";

    char pem_text[1024] = { 0 };
    int max_encoded_length = Base64encode_len(this->public_key_length);
    int result = Base64encode(pem_text, (const char*)this->public_key, this->public_key_length);

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

    if (this->debug)
    {
        log_debug("Public Key Info: ");
        BIO* keybio = BIO_new(BIO_s_mem());
        RSA_print(keybio, rsa_public_key, 0);
        char buffer[2048] = { 0 };
        std::string res = "";
        while (BIO_read(keybio, buffer, 2048) > 0)
        {
            log_debug(buffer);
        }
        BIO_free(keybio);
    }

    unsigned char encrypted_shared_secret[256] = { 0 };
    int encrypted_shared_secret_len = RSA_public_encrypt(this->shared_secret.length(), (unsigned char*)this->shared_secret.c_str(), encrypted_shared_secret, rsa_public_key, RSA_PKCS1_PADDING);
    log_debug("Encrypted shared secret with public key");

    unsigned char encrypted_verify_token[256] = { 0 };
    int encrypted_verify_token_len = RSA_public_encrypt(this->verify_token_length, this->verify_token, encrypted_verify_token, rsa_public_key, RSA_PKCS1_PADDING);
    log_debug("Encrypted verify token with public key");

    uint8_t packet[1028];
    size_t offset = 0;

    PacketEncoder::write_var_int(0x01, packet, offset); // packet id

    PacketEncoder::write_var_int(encrypted_shared_secret_len, packet, offset); // shared secret length
    PacketEncoder::write_byte_array(encrypted_shared_secret, encrypted_shared_secret_len, packet, offset); // shared secret

    PacketEncoder::write_var_int(encrypted_verify_token_len, packet, offset); // verify token length
    PacketEncoder::write_byte_array(encrypted_verify_token, encrypted_verify_token_len, packet, offset); // verify token

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        log_error("Failed to send packet");
        print_winsock_error();
    }
    else
    {
        this->encryption_enabled = true;
        this->sock.initialize_encryption((unsigned char*)this->shared_secret.c_str(), (unsigned char*)this->shared_secret.c_str());
        log_debug("Sent encryption response - ENCRYPTION ENABLED");
    }
}

void mcbot::MCBot::send_keep_alive(int id)
{
    log_debug(">>> Sending PacketPlayInKeepAlive...");

    int packet_id = 0x00;
    uint8_t* packet = new uint8_t[PacketEncoder::get_var_int_size(id) + PacketEncoder::get_var_int_size(packet_id)];
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset); // packet id
    PacketEncoder::write_var_int(id, packet, offset);

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}

void mcbot::MCBot::send_chat_message(std::string message)
{
    log_debug(">>> Sending PacketPlayInChat...");

    int packet_id = 0x01;
    uint8_t* packet = new uint8_t[message.length() + PacketEncoder::get_var_int_size(message.length()) + 1 + PacketEncoder::get_var_int_size(packet_id)]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset); 
    PacketEncoder::write_string(message, packet, offset);

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        log_error("Failed to send packet");
        print_winsock_error();
    }
    else
    {
        log_debug("Message: " + message);
    }

    delete[] packet;
}

void mcbot::MCBot::send_position(Vector<double> position, bool on_ground)
{
    log_debug(">>> Sending PacketPlayInPosition...");


    int packet_id = 0x04;
    uint8_t* packet = new uint8_t[1024]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_double(position.get_x(), packet, offset);
    PacketEncoder::write_double(position.get_y(), packet, offset);
    PacketEncoder::write_double(position.get_z(), packet, offset);
    PacketEncoder::write_boolean(on_ground, packet, offset);

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;

    // Internal updates
    this->player.update_location(position);
}

void mcbot::MCBot::send_look(float yaw, float pitch, bool on_ground)
{
    log_debug(">>> Sending PacketPlayInLook...");

    int packet_id = 0x05;
    uint8_t* packet = new uint8_t[1024]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_float(yaw, packet, offset);
    PacketEncoder::write_float(pitch, packet, offset);
    PacketEncoder::write_boolean(on_ground, packet, offset);

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;

    // Internal updates
    this->player.update_rotation(yaw, pitch);
}

void mcbot::MCBot::send_position_look(Vector<double> position, float yaw, float pitch, bool on_ground)
{
    log_debug(">>> Sending PacketPlayInPositionLook...");

    int packet_id = 0x06;
    uint8_t* packet = new uint8_t[1024]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_double(position.get_x(), packet, offset);
    PacketEncoder::write_double(position.get_y(), packet, offset);
    PacketEncoder::write_double(position.get_z(), packet, offset);
    PacketEncoder::write_float(yaw, packet, offset);
    PacketEncoder::write_float(pitch, packet, offset);
    PacketEncoder::write_boolean(on_ground, packet, offset);

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;

    // Internal updates
    this->player.update_location(position);
    this->player.update_rotation(yaw, pitch);
}

void mcbot::MCBot::send_held_item_slot(short slot)
{
    log_debug(">>> Sending PacketPlayInHeldItemSlot...");

    int packet_id = 0x09;
    uint8_t* packet = new uint8_t[1024]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_short(slot, packet, offset);

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}

void mcbot::MCBot::send_settings()
{
    log_debug(">>> Sending PacketPlayInSettings...");

    int packet_id = 0x15;
    uint8_t* packet = new uint8_t[1024]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_string("en_us", packet, offset);
    PacketEncoder::write_byte(16, packet, offset); // render distance
    PacketEncoder::write_byte(0, packet, offset); // chat mode
    PacketEncoder::write_boolean(true, packet, offset); // chat colors
    PacketEncoder::write_byte(0x7F, packet, offset); // skin parts

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}

void mcbot::MCBot::send_custom_payload(std::string message)
{
    log_debug(">>> Sending PacketPlayInCustomPayload...");

    int packet_id = 0x17;
    uint8_t* packet = new uint8_t[1024]{ 0 };
    size_t offset = 0;

    PacketEncoder::write_var_int(packet_id, packet, offset);
    PacketEncoder::write_string(message, packet, offset);

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        log_error("Failed to send packet");
        print_winsock_error();
    }

    delete[] packet;
}


void mcbot::MCBot::recv_packet()
{
    int length = this->read_next_var_int();
    log_debug("Length: " + std::to_string(length));

    uint8_t* packet;
    int decompressed_length = 0;
    if (this->compression_enabled)
    {
        decompressed_length = this->read_next_var_int();
        length -= PacketEncoder::get_var_int_size(decompressed_length);
        log_debug("Decompressed length: " + std::to_string(decompressed_length));
    }

    int packet_size;
    if (decompressed_length == 0)
    {
        packet = (uint8_t*)calloc(length, sizeof(uint8_t));
        packet_size = this->read_next_packet(length, packet);
    }
    else
    {
        packet = (uint8_t*)calloc(decompressed_length, sizeof(uint8_t));
        packet_size = this->read_next_packet(length, packet, decompressed_length);
    }

    size_t offset = 0;
    int packet_id = PacketDecoder::read_var_int(packet, offset);
    log_debug("ID: " + std::to_string(packet_id));

    handle_recv_packet(packet_id, packet, packet_size, offset);

    free(packet);
}

void mcbot::MCBot::handle_recv_packet(int packet_id, uint8_t* packet, int length, size_t& offset)
{
    if (this->state == State::HANDSHAKE)
    {

    }
    else if (this->state == State::STATUS)
    {

    }
    else if (this->state == State::LOGIN)
    {
        switch (packet_id)
        {
            case 0x00:
                this->recv_login_disconnect(packet, length, offset);
                break;
            case 0x01:
                this->recv_encryption_request(packet, length, offset);
                break;
            case 0x02:
                this->recv_login_success(packet, length, offset);
                break;
            case 0x03:
                this->recv_set_compression(packet, length, offset);
                break;
            default:
                log_error("Unhandled " + mcbot::to_string(this->state) + " packet id: " + std::to_string(packet_id));
        }
    }
    else if (this->state == State::PLAY)
    {
        switch (packet_id)
        {
            case 0x00:
                this->recv_keep_alive(packet, length, offset);
                break;
            case 0x01:
                this->recv_join_server(packet, length, offset);
                break;
            case 0x02:
                this->recv_chat_message(packet, length, offset);
                break;
            case 0x03:
                this->recv_update_time(packet, length, offset);
                break;
            case 0x04:
                this->recv_entity_equipment(packet, length, offset);
                break;
            case 0x05:
                this->recv_spawn_position(packet, length, offset);
                break;
            case 0x06:
                this->recv_update_health(packet, length, offset);
                break;
            case 0x08:
                this->recv_position(packet, length, offset);
                break;
            case 0x09:
                this->recv_held_item_slot(packet, length, offset);
                break;
            case 0x0A:
                this->recv_bed(packet, length, offset);
                break;
            case 0x0B:
                this->recv_animation(packet, length, offset);
                break;
            case 0x0C:
                this->recv_named_entity_spawn(packet, length, offset);
                break;
            case 0x0D:
                this->recv_collect(packet, length, offset);
                break;
            case 0x0E:
                this->recv_spawn_entity(packet, length, offset);
                break;
            case 0x0F:
                this->recv_spawn_entity_living(packet, length, offset);
                break;
            case 0x10:
                this->recv_spawn_entity_painting(packet, length, offset);
                break;
            case 0x11:
                this->recv_spawn_entity_experience_orb(packet, length, offset);
                break;
            case 0x12:
                this->recv_entity_velocity(packet, length, offset);
                break;
            case 0x13:
                this->recv_entity_destroy(packet, length, offset);
                break;
            case 0x14:
                this->recv_entity(packet, length, offset);
                break;
            case 0x15:
                this->recv_rel_entity_move(packet, length, offset);
                break;
            case 0x16:
                this->recv_entity_look(packet, length, offset);
                break;
            case 0x17:
                this->recv_rel_entity_move_look(packet, length, offset);
                break;
            case 0x18:
                this->recv_entity_teleport(packet, length, offset);
                break;
            case 0x19:
                this->recv_entity_head_look(packet, length, offset);
                break;
            case 0x1A:
                this->recv_entity_status(packet, length, offset);
                break;
            case 0x1C:
                this->recv_entity_metadata(packet, length, offset);
                break;
            case 0x1D:
                this->recv_entity_effect(packet, length, offset);
                break;
            case 0x1F:
                this->recv_experience(packet, length, offset);
                break;
            case 0x20:
                this->recv_entity_attributes(packet, length, offset);
                break;
            case 0x21:
                this->recv_map_chunk(packet, length, offset);
                break;
            case 0x22:
                this->recv_multi_block_change(packet, length, offset);
                break;
            case 0x23:
                this->recv_block_change(packet, length, offset);
                break;
            case 0x25:
                this->recv_block_break(packet, length, offset);
                break;
            case 0x28:
                this->recv_world_event(packet, length, offset);
                break;
            case 0x26:
                this->recv_map_chunk_bulk(packet, length, offset);
                break;
            case 0x29:
                this->recv_named_sound_effect(packet, length, offset);
                break;
            case 0x2A:
                this->recv_world_particles(packet, length, offset);
                break;
            case 0x2B:
                this->recv_game_state_change(packet, length, offset);
                break;
            case 0x2F:
                this->recv_set_slot(packet, length, offset);
                break;
            case 0x30:
                this->recv_window_items(packet, length, offset);
                break;
            case 0x32:
                this->recv_transaction(packet, length, offset);
                break;
            case 0x33:
                this->recv_update_sign(packet, length, offset);
                break;
            case 0x35:
                this->recv_tile_entity_data(packet, length, offset);
                break;
            case 0x37:
                this->recv_statistics(packet, length, offset);
                break;
            case 0x38:
                this->recv_player_info(packet, length, offset);
                break;
            case 0x39:
                this->recv_abilities(packet, length, offset);
                break;
            case 0x3B:
                this->recv_scoreboard_objective(packet, length, offset);
                break;
            case 0x3C:
                this->recv_update_scoreboard_score(packet, length, offset);
                break;
            case 0x3D:
                this->recv_display_scoreboard(packet, length, offset);
                break;
            case 0x3E:
                this->recv_scoreboard_team(packet, length, offset);
                break;
            case 0x3F:
                this->recv_plugin_message(packet, length, offset);
                break;
            case 0x40:
                this->recv_play_disconnect(packet, length, offset);
                break;
            case 0x41:
                this->recv_server_difficulty(packet, length, offset);
                break;
            case 0x44:
                this->recv_world_border(packet, length, offset);
                break;
            case 0x45:
                this->recv_title(packet, length, offset);
                break;
            case 0x47:
                this->recv_player_list_header_footer(packet, length, offset);
                break;
            default:
                log_error("Unhandled " + mcbot::to_string(this->state) + " packet id: " + std::to_string(packet_id));
        }
    }
    else
    {
        log_error("Unknown state: " + mcbot::to_string(this->state));
    }
}

void mcbot::MCBot::recv_login_disconnect(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketLoginOutDisconnect...");

    std::cout << packet << std::endl;
    
    this->connected = false;
    log_info("Disconnected");
}

void mcbot::MCBot::recv_set_compression(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketLoginOutSetCompression...");
    this->max_uncompressed_length = PacketDecoder::read_var_int(packet, offset);

    log_debug("\tMax Uncompressed Length: " + std::to_string(this->max_uncompressed_length));
    this->compression_enabled = true;
    this->sock.initialize_compression(this->max_uncompressed_length);
    log_debug("<<< COMPRESSION ENABLED <<<");
}

void mcbot::MCBot::recv_encryption_request(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketLoginOutEncryptionRequest...");

    // Server ID //
    this->server_id = PacketDecoder::read_string(packet, offset);

    // Public Key //
    this->public_key_length = PacketDecoder::read_var_int(packet, offset);
    this->public_key = new uint8_t[public_key_length];
    PacketDecoder::read_byte_array(this->public_key, public_key_length, packet, offset);

    // Verify Token //
    this->verify_token_length = PacketDecoder::read_var_int(packet, offset);
    this->verify_token = new uint8_t[verify_token_length];
    PacketDecoder::read_byte_array(this->verify_token, verify_token_length, packet, offset);

    // Save Session //
    // - So Yggdrasil authentication doesn't kick us!
    log_debug("Saving session...");
    if (this->send_session() < 0)
    {
        log_error("Invalid session!");
    }
    else
    {
        this->send_encryption_response();
    }
}

void mcbot::MCBot::recv_login_success(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayLoginOutSuccess...");
    this->state = State::PLAY;

    std::string uuid_string = PacketDecoder::read_string(packet, offset);
    std::string username = PacketDecoder::read_string(packet, offset);
    
    log_debug(
        "UUID: " + uuid_string + '\n' +
        "\tUsername: " + username);

    log_info("Successfully logged in!");
}

void mcbot::MCBot::recv_play_disconnect(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutKickDisconnect...");

    std::string reason = PacketDecoder::read_string(packet, offset);
    this->connected = false;

    log_info("Disconnected: " + reason);
}

void mcbot::MCBot::recv_keep_alive(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutKeepAlive...");

    int id = PacketDecoder::read_var_int(packet, offset);

    send_keep_alive(id);

    this->ready = true;
}

void mcbot::MCBot::recv_join_server(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutJoinServer...");

    int entity_id = PacketDecoder::read_int(packet, offset);
    mcbot::Gamemode gamemode = (mcbot::Gamemode) PacketDecoder::read_byte(packet, offset);
    mcbot::Dimension dimension = (mcbot::Dimension) PacketDecoder::read_byte(packet, offset);
    mcbot::Difficulty difficulty = (mcbot::Difficulty) PacketDecoder::read_byte(packet, offset);
    uint8_t max_players = PacketDecoder::read_byte(packet, offset);
    std::string level_type = PacketDecoder::read_string(packet, offset);
    bool reduced_debug_info = PacketDecoder::read_boolean(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) + 
        "\n\tGamemode: " + mcbot::to_string(gamemode) +
        "\n\tDimension: " + mcbot::to_string(dimension) +
        "\n\tDifficulty: " + mcbot::to_string(difficulty) +
        "\n\tMax Players: " + std::to_string((int)max_players) +
        "\n\tLevel Type: " + level_type +
        "\n\tReduced Debug Info: " + mcbot::to_string(reduced_debug_info));
}

void mcbot::MCBot::recv_chat_message(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutChat...");

    std::string chat_data = PacketDecoder::read_string(packet, offset);
    mcbot::ChatPosition position = (mcbot::ChatPosition) PacketDecoder::read_byte(packet, offset);

    log_debug(
        "Chat Data: " + chat_data +
        "\n\tPosition: " + mcbot::to_string(position));


    auto chat_json = nlohmann::json::parse(chat_data);

    std::string chat_message = chat_json["text"];
    for (auto var : chat_json["extra"])
    {
        if (var.contains("text"))
        {
            chat_message += var["text"];
        }
        else
        {
            chat_message += var;
        }
    }
    boost::erase_all(chat_message, "\"");

    log_chat(chat_message);
}

void mcbot::MCBot::recv_update_time(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutUpdateTime...");

    int64_t world_age = (int64_t) PacketDecoder::read_long(packet, offset);
    int64_t time_of_day = (int64_t) PacketDecoder::read_long(packet, offset);

    log_debug(
        "World Age: " + std::to_string(world_age) +
        "\n\tTime of Day: " + std::to_string(time_of_day));
}

void mcbot::MCBot::recv_entity_equipment(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutEntityEquipment...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    short slot = PacketDecoder::read_short(packet, offset);
    mcbot::Slot item = PacketDecoder::read_slot(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tSlot: " + std::to_string(slot) +
        "\n\tItem: " + item.to_string());
}

void mcbot::MCBot::recv_spawn_position(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutSpawnPosition...");

    mcbot::Position location = PacketDecoder::read_position(packet, offset);

    log_debug("Location: " + location.to_string());
}

void mcbot::MCBot::recv_update_health(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutUpdateHealth...");

    float health = PacketDecoder::read_float(packet, offset);
    int food = PacketDecoder::read_var_int(packet, offset);
    float food_saturation = PacketDecoder::read_float(packet, offset);

    log_debug("Health: " + std::to_string(health) + 
        "\n\tFood: " + std::to_string(food) +
        "\n\tSaturation: " + std::to_string(food_saturation));
}


void mcbot::MCBot::recv_position(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutPosition...");

    mcbot::Vector<double> position = PacketDecoder::read_vector<double>(packet, offset);

    this->player.update_location(position);

    float yaw = PacketDecoder::read_float(packet, offset);
    float pitch = PacketDecoder::read_float(packet, offset);
    uint8_t flags = PacketDecoder::read_byte(packet, offset);

    log_debug("Position: " + position.to_string() +
        "\n\tYaw: " + std::to_string(yaw) +
        "\n\tPitch: " + std::to_string(pitch) +
        "\n\tFlags: " + std::to_string((int)flags));
}

void mcbot::MCBot::recv_held_item_slot(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutHeldItemSlot...");

    uint8_t held_item_slot = PacketDecoder::read_byte(packet, offset);

    log_debug("Held Item Slot: " + std::to_string((int) held_item_slot));
}

void mcbot::MCBot::recv_bed(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutBed...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Position position = PacketDecoder::read_position(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tLocation: " + position.to_string());
}

void mcbot::MCBot::recv_animation(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutAnimation...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Position position = PacketDecoder::read_position(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tLocation: " + position.to_string());
}

void mcbot::MCBot::recv_named_entity_spawn(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutNamedEntitySpawn...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    UUID uuid = PacketDecoder::read_uuid(packet, offset);
    mcbot::Vector<int> position = mcbot::Vector<int>(PacketDecoder::read_int(packet, offset), PacketDecoder::read_int(packet, offset), PacketDecoder::read_int(packet, offset));
    uint8_t pitch = PacketDecoder::read_byte(packet, offset);
    uint8_t yaw = PacketDecoder::read_byte(packet, offset);
    short current_item = PacketDecoder::read_short(packet, offset);
    EntityMetaData meta_data = PacketDecoder::read_meta_data(packet, offset);

    if (this->uuid_to_player.count(uuid) == 0)
    {
        EntityPlayer player = EntityPlayer(entity_id, uuid);
        player.update_location(position);
        player.update_rotation(yaw, pitch);

        this->uuid_to_player.insert(std::pair<UUID, EntityPlayer>(uuid, player));
    }
    else
    {
        EntityPlayer player = this->uuid_to_player.at(uuid);
        player.set_id(entity_id);
        player.update_location(position);
        player.update_rotation(yaw, pitch);
    }

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tUUID: " + uuid.to_string() + 
        "\n\tLocation: " + position.to_string());
}

void mcbot::MCBot::recv_collect(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutCollect...");

    int collected_id = PacketDecoder::read_var_int(packet, offset);
    int collector_id = PacketDecoder::read_var_int(packet, offset);

    log_debug(
        "Collected ID: " + std::to_string(collected_id) +
        "\n\tCollector ID: " + std::to_string(collector_id));
}

void mcbot::MCBot::recv_spawn_entity(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutSpawnEntity...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    EntityType type = (EntityType)PacketDecoder::read_byte(packet, offset);
    mcbot::Vector<int> position = PacketDecoder::read_vector<int>(packet, offset);
    position.scale(1 / 32.0);

    uint8_t pitch = PacketDecoder::read_byte(packet, offset);
    uint8_t yaw = PacketDecoder::read_byte(packet, offset);
    int data = PacketDecoder::read_int(packet, offset);

    if (data > 0)
    {
        mcbot::Vector<short> motion = PacketDecoder::read_vector<short>(packet, offset);
    }

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tLocation:" + position.to_string());

    this->entities.push_back(Entity(type, entity_id, position));
}

void mcbot::MCBot::recv_spawn_entity_living(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutSpawnEntityLiving...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    EntityType type = (EntityType)PacketDecoder::read_byte(packet, offset);
    mcbot::Vector<int> position = PacketDecoder::read_vector<int>(packet, offset);
    position.scale(1 / 32.0);

    uint8_t pitch = PacketDecoder::read_byte(packet, offset);
    uint8_t yaw = PacketDecoder::read_byte(packet, offset);
    uint8_t head_pitch = PacketDecoder::read_byte(packet, offset);
    mcbot::Vector<short> motion = PacketDecoder::read_vector<short>(packet, offset);
    EntityMetaData meta_data = PacketDecoder::read_meta_data(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tLocation:" + position.to_string());

    this->entities.push_back(EntityLiving(type, entity_id, position));
}

void mcbot::MCBot::recv_spawn_entity_painting(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutSpawnEntityPainting...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    std::string title = PacketDecoder::read_string(packet, offset);
    mcbot::Position location = PacketDecoder::read_position(packet, offset);
    uint8_t direction = PacketDecoder::read_byte(packet, offset);
    
    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tLocation: " + location.to_string() +
        "\n\tTitle: " + title + 
        "\n\tDirection: " + std::to_string((int)direction));
}

void mcbot::MCBot::recv_spawn_entity_experience_orb(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutSpawnEntityExperienceOrb...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Vector<int> motion = PacketDecoder::read_vector<int>(packet, offset);
    short count = PacketDecoder::read_short(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tPosition: " + motion.to_string() + 
        "\n\tCount: " + std::to_string(count));
}

void mcbot::MCBot::recv_entity_velocity(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutEntityVelocity...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Vector<short> motion = PacketDecoder::read_vector<short>(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tVelocity: " + motion.to_string());
}

void mcbot::MCBot::recv_entity_destroy(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutEntityDestroy...");

    int count = PacketDecoder::read_var_int(packet, offset);

    mcbot::Buffer<int> entity_ids = mcbot::Buffer<int>(count);
    for (int i = 0; i < count; i++)
    {
        entity_ids.put(PacketDecoder::read_var_int(packet, offset));
    }

    log_debug(
        "Entity Count: " + std::to_string(count) +
        "\n\tEntities: " + entity_ids.to_string());
}

void mcbot::MCBot::recv_entity(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutEntity...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id));
}

void mcbot::MCBot::recv_rel_entity_move(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutRelEntityMove...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Vector<uint8_t> dmot = PacketDecoder::read_vector<uint8_t>(packet, offset);
    bool on_ground = PacketDecoder::read_boolean(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tChange in Motion: " + dmot.to_string() + 
        "\n\tOn Ground: " + std::to_string(on_ground));
}

void mcbot::MCBot::recv_entity_look(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutEntityLook...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    uint8_t yaw = PacketDecoder::read_byte(packet, offset);
    uint8_t pitch = PacketDecoder::read_byte(packet, offset);
    bool on_ground = PacketDecoder::read_boolean(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tYaw: " + std::to_string((int)yaw) +
        "\n\tPitch: " + std::to_string((int)pitch) +
        "\n\tOn Ground: " + std::to_string(on_ground));
}

void mcbot::MCBot::recv_rel_entity_move_look(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutRelEntityMoveLook...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Vector<uint8_t> dmot = PacketDecoder::read_vector<uint8_t>(packet, offset);
    uint8_t yaw = PacketDecoder::read_byte(packet, offset);
    uint8_t pitch = PacketDecoder::read_byte(packet, offset);
    bool on_ground = PacketDecoder::read_boolean(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tChange in Motion: " + dmot.to_string() +
        "\n\tYaw: " + std::to_string((int)yaw) +
        "\n\tPitch: " + std::to_string((int)pitch) +
        "\n\tOn Ground: " + std::to_string(on_ground));
}

void mcbot::MCBot::recv_entity_teleport(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutEntityTeleport...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Vector<int> position = PacketDecoder::read_vector<int>(packet, offset);
    uint8_t yaw = PacketDecoder::read_byte(packet, offset);
    uint8_t pitch = PacketDecoder::read_byte(packet, offset);
    bool on_ground = PacketDecoder::read_boolean(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tChange in Motion: " + position.to_string() +
        "\n\tYaw: " + std::to_string((int)yaw) + 
        "\n\tPitch: " + std::to_string((int)pitch) +
        "\n\tOn Ground: " + std::to_string(on_ground));
}

void mcbot::MCBot::recv_entity_head_look(uint8_t* packet, size_t length, size_t& offset)
{
    // TODO: find actual name of packet
    log_debug("<<< Handling PacketPlayOutEntityHeadLook...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    uint8_t angle = PacketDecoder::read_byte(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tAngle: " + std::to_string((int)angle));
}

void mcbot::MCBot::recv_entity_status(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutEntityStatus...");

    int entity_id = PacketDecoder::read_int(packet, offset);
    mcbot::EntityStatus status = (mcbot::EntityStatus) PacketDecoder::read_byte(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tStatus: " + mcbot::to_string(status));
}

void mcbot::MCBot::recv_entity_metadata(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutEntityMetadata...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::EntityMetaData meta_data = PacketDecoder::read_meta_data(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tMeta Data: size " + std::to_string(meta_data.get_values().size()));
}

void mcbot::MCBot::recv_entity_effect(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutEntityEffect...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    uint8_t effect_id = PacketDecoder::read_byte(packet, offset);
    uint8_t amplifier = PacketDecoder::read_byte(packet, offset);
    int duration = PacketDecoder::read_var_int(packet, offset);
    bool hide_particles = PacketDecoder::read_boolean(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tEffect ID: " + std::to_string((int)effect_id) +
        "\n\tAmplifier: " + std::to_string((int)amplifier) +
        "\n\tDuration: " + std::to_string(duration) +
        "\n\tHide Particles: " + mcbot::to_string(hide_particles));
}

void mcbot::MCBot::recv_experience(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutExperience...");

    float experience_bar = PacketDecoder::read_float(packet, offset);
    int level = PacketDecoder::read_var_int(packet, offset);
    int total_experience = PacketDecoder::read_var_int(packet, offset);

    log_debug(
        "Experience Bar: " + std::to_string(experience_bar) +
        "\n\tLevel: " + std::to_string(level) +
        "\n\tTotal Experience: " + std::to_string(total_experience));
}

void mcbot::MCBot::recv_entity_attributes(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutUpdateAttributes...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    int num_attributes = PacketDecoder::read_int(packet, offset);

    std::list<mcbot::Attribute> attributes;
    for (int i = 0; i < num_attributes; i++)
    {
        attributes.push_back(PacketDecoder::read_attribute(packet, offset));
    }

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tAttributes: " + std::to_string(attributes.size()));

}

void mcbot::MCBot::recv_map_chunk(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutMapChunk...");

    int chunk_x = PacketDecoder::read_int(packet, offset);
    int chunk_z = PacketDecoder::read_int(packet, offset);
    bool ground_up_continuous = PacketDecoder::read_boolean(packet, offset);
    uint16_t primary_bitmask = PacketDecoder::read_short(packet, offset);
    int data_size = PacketDecoder::read_var_int(packet, offset);

    Chunk chunk = PacketDecoder::read_chunk(chunk_x, chunk_z, ground_up_continuous, true, primary_bitmask, packet, offset);
    this->load_chunk(chunk);

    log_debug(
        "X: " + std::to_string(chunk_x) +
        "\n\tZ: " + std::to_string(chunk_z));
}

void mcbot::MCBot::recv_multi_block_change(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutMultiBlockChange...");

    int chunk_x = PacketDecoder::read_int(packet, offset);
    int chunk_z = PacketDecoder::read_int(packet, offset);
    int record_count = PacketDecoder::read_var_int(packet, offset);

    for (int i = 0; i < record_count; i++)
    {
        uint8_t horizontal_position = PacketDecoder::read_byte(packet, offset);
        uint8_t y_coordinate = PacketDecoder::read_byte(packet, offset);
        int block_id = PacketDecoder::read_var_int(packet, offset);
    }

    log_debug(
        "X: " + std::to_string(chunk_x) +
        "\n\tZ: " + std::to_string(chunk_z));
}

void mcbot::MCBot::recv_block_change(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutBlockChange...");

    mcbot::Position location = PacketDecoder::read_position(packet, offset);
    int block_id = PacketDecoder::read_var_int(packet, offset);

    log_debug(
        "Location: " + location.to_string() +
        "\n\tBlock ID: " + std::to_string(block_id));
}

void mcbot::MCBot::recv_block_break(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutBlockBreak...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Position location = PacketDecoder::read_position(packet, offset);
    uint8_t destroy_stage = PacketDecoder::read_byte(packet, offset);

    log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tLocation: " + location.to_string() +
        "\n\tDestroy Stage: " + std::to_string((int)destroy_stage));
}

int chunk_data_size(int num_sections, bool skylight_sent, bool ground_up_continuous)
{
    int block_ids_size = num_sections * 2 * 16 * 16 * 16;
    int emitted_light_size = num_sections * 16 * 16 * 16 / 2;
    int sky_light_size = skylight_sent ? num_sections * 16 * 16 * 16 / 2 : 0;
    int ground_up_continuous_size = ground_up_continuous ? 256 : 0;

    return block_ids_size + emitted_light_size + sky_light_size + ground_up_continuous_size;
}

void mcbot::MCBot::recv_map_chunk_bulk(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutMapChunkBulk...");

    bool sky_light_sent = PacketDecoder::read_boolean(packet, offset);
    int chunk_column_count = PacketDecoder::read_var_int(packet, offset);
    std::list<Chunk> chunks = std::list<Chunk>();
    
    for (int i = 0; i < chunk_column_count; i++)
    {
        int x = PacketDecoder::read_int(packet, offset);
        int z = PacketDecoder::read_int(packet, offset);
        uint16_t primary_bit_mask = PacketDecoder::read_short(packet, offset);
        chunks.push_back(Chunk(x, z, primary_bit_mask));
        log_debug("Loading Chunk (" + std::to_string(x) + "," + std::to_string(z) + ")");
    }

    for (Chunk chunk : chunks)
    {
        PacketDecoder::read_chunk_bulk(chunk, sky_light_sent, packet, offset);
        this->load_chunk(chunk);
    }
}

void mcbot::MCBot::recv_world_event(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutWorldEvent...");

    int effect_id = PacketDecoder::read_int(packet, offset);
    mcbot::Position location = PacketDecoder::read_position(packet, offset);
    int data = PacketDecoder::read_int(packet, offset);
    bool disable_relative_volume = PacketDecoder::read_boolean(packet, offset);

    log_debug(
        "Effect ID: " + std::to_string(effect_id) +
        "\n\tPosition: " + location.to_string());
}

void mcbot::MCBot::recv_named_sound_effect(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutNamedSoundEffect...");

    std::string sound_name = PacketDecoder::read_string(packet, offset);
    mcbot::Vector<int> position = PacketDecoder::read_vector<int>(packet, offset);
    float volume = PacketDecoder::read_float(packet, offset);
    uint8_t pitch = PacketDecoder::read_byte(packet, offset);

    log_debug(
        "Sound Name: " + sound_name + 
        "\n\tPosition: " + position.to_string());
}

void mcbot::MCBot::recv_world_particles(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutWorldParticles...");

    mcbot::ParticleType particle_id = (mcbot::ParticleType) PacketDecoder::read_int(packet, length);
    bool long_distance = PacketDecoder::read_boolean(packet, length);
    mcbot::Vector<float> position = PacketDecoder::read_vector<float>(packet, length);
    mcbot::Vector<float> offset_vec = PacketDecoder::read_vector<float>(packet, length);
    float particle_data = PacketDecoder::read_float(packet, length);
    int particle_count = PacketDecoder::read_int(packet, length);

    int data_length = 0;
    if (particle_id == mcbot::ParticleType::ICON_CRACK)
    {
        data_length = 2;
    }
    else if (particle_id == mcbot::ParticleType::BLOCK_CRACK || particle_id == mcbot::ParticleType::BLOCK_DUST)
    {
        data_length = 1;
    }

    mcbot::Buffer<int> data = PacketDecoder::read_var_int_array(data_length, packet, offset);

    log_debug(
        "Particle ID: " + mcbot::to_string(particle_id) +
        "\n\tLong Distance: " + std::to_string(long_distance) +
        "\n\tPosition: " + position.to_string() +
        "\n\tOffset: " + offset_vec.to_string() +
        "\n\tParticle Data: " + std::to_string(particle_data)
    );
}

void mcbot::MCBot::recv_game_state_change(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutGameStateChange...");

    uint8_t reason = PacketDecoder::read_byte(packet, offset);
    float value = PacketDecoder::read_float(packet, offset);

    log_debug(
        "Reason: " + std::to_string((int)reason) +
        "\n\tValue: " + std::to_string(value));
}

void mcbot::MCBot::recv_set_slot(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutSetSlot...");

    uint8_t window_id = PacketDecoder::read_byte(packet, offset);
    uint16_t slot_number = PacketDecoder::read_short(packet, offset);
    mcbot::Slot slot = PacketDecoder::read_slot(packet, offset);

    log_debug(
        "Window ID: " + std::to_string(window_id) +
        "\n\tSlot Number: " + std::to_string(slot_number) +
        "\n\tSlot: " + slot.to_string()
    );
}

void mcbot::MCBot::recv_window_items(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutWindowItems...");

    uint8_t window_id = PacketDecoder::read_byte(packet, offset);
    int16_t count = PacketDecoder::read_short(packet, offset);
    std::list<mcbot::Slot> slots = PacketDecoder::read_slot_array(count, packet, offset);

    log_debug(
        "Window ID: " + std::to_string(window_id) +
        "\n\tCount: " + std::to_string(count)
    );
}

void mcbot::MCBot::recv_transaction(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutTransaction...");

    uint8_t window_id = PacketDecoder::read_byte(packet, offset);
    int16_t action_number = PacketDecoder::read_short(packet, offset);
    bool accepted = PacketDecoder::read_boolean(packet, offset);

    log_debug(
        "Window ID: " + std::to_string(window_id) +
        "\n\tAction Number: " + std::to_string(action_number) +
        "\n\tAccept: " + std::to_string(accepted)
    );
}

void mcbot::MCBot::recv_update_sign(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutUpdateSign...");

    mcbot::Position location = PacketDecoder::read_position(packet, offset);
    std::string line1 = PacketDecoder::read_string(packet, offset);
    std::string line2 = PacketDecoder::read_string(packet, offset);
    std::string line3 = PacketDecoder::read_string(packet, offset);
    std::string line4 = PacketDecoder::read_string(packet, offset);

    log_debug(
        "Location: " + location.to_string() +
        "\n\tLine 1: " + line1 +
        "\n\tLine 2: " + line2 +
        "\n\tLine 3: " + line3 +
        "\n\tLine 4: " + line4
    );
}

void mcbot::MCBot::recv_tile_entity_data(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutTileEntityData...");

    mcbot::Position location = PacketDecoder::read_position(packet, offset);
    mcbot::TileEntityAction action = (mcbot::TileEntityAction) PacketDecoder::read_byte(packet, offset);
    mcbot::NBT nbt = PacketDecoder::read_nbt(packet, offset);

    log_debug(
        "Location: " + location.to_string() +
        "\n\tAction: " + mcbot::to_string(action) +
        "\n\tNBT: " + nbt.to_string()
    );
}

void mcbot::MCBot::recv_statistics(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutStatistics...");
    int count = PacketDecoder::read_var_int(packet, offset);
    std::list<mcbot::Statistic> statistics = PacketDecoder::read_statistic_array(count, packet, offset);

    log_debug("Statistics (" + std::to_string(count) + "): ");

    for (auto statistic : statistics)
    {
        log_debug("\t" + statistic.to_string());
    }
}

void mcbot::MCBot::recv_player_info(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutPlayerInfo...");

    mcbot::PlayerInfoAction action = (mcbot::PlayerInfoAction) PacketDecoder::read_var_int(packet, offset);
    int players_length = PacketDecoder::read_var_int(packet, offset);
    update_player_info(action, players_length, packet, offset);

}

void mcbot::MCBot::recv_abilities(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutAbilities...");
    uint8_t flags = PacketDecoder::read_byte(packet, offset);
    float flying_speed = PacketDecoder::read_float(packet, offset);
    float fov_modifier = PacketDecoder::read_float(packet, offset);

    log_debug(
        "Flags: " + std::to_string((int)flags) +
        "\n\tFlying Speed: " + std::to_string(flying_speed) +
        "\n\tFOV Modifier: " + std::to_string(fov_modifier));
}

void mcbot::MCBot::recv_scoreboard_objective(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutScoreboardObjective...");
    std::string objective_name = PacketDecoder::read_string(packet, offset);
    mcbot::ScoreboardMode mode = (mcbot::ScoreboardMode) PacketDecoder::read_byte(packet, offset);

    log_debug(
        "Objective Name: " + objective_name +
        "\n\tMode: " + mcbot::to_string(mode));

    if (mode == mcbot::ScoreboardMode::CREATE ||
        mode == mcbot::ScoreboardMode::UPDATE)
    {
        std::string objective_value = PacketDecoder::read_string(packet, offset);
        std::string objective_type = PacketDecoder::read_string(packet, offset);
        log_debug(
            "Objective value: " + objective_value +
            "\n\tObjective type: " + objective_type);
    }
}

void mcbot::MCBot::recv_update_scoreboard_score(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutScoreboardScore...");
    std::string score_name = PacketDecoder::read_string(packet, offset);
    mcbot::ScoreAction action = (mcbot::ScoreAction) PacketDecoder::read_byte(packet, offset);
    std::string objective_name = PacketDecoder::read_string(packet, offset);

    log_debug(
        "Score Name: " + score_name +
        "\n\tAction: " + mcbot::to_string(action) +
        "\n\tObjective Name: " + objective_name);

    if (action != mcbot::ScoreAction::REMOVE)
    {
        int value = PacketDecoder::read_var_int(packet, offset);
        log_debug("\n\tValue: " + std::to_string(value));
    }

}

void mcbot::MCBot::recv_display_scoreboard(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutScoreboardDisplayObjective...");

    mcbot::ScoreboardPosition position = (mcbot::ScoreboardPosition) PacketDecoder::read_byte(packet, offset);
    std::string score_name = PacketDecoder::read_string(packet, offset);

    log_debug(
        "Position: " + mcbot::to_string(position) +
        "\n\tScore Name: " + score_name);
}

void mcbot::MCBot::recv_scoreboard_team(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutScoreboardTeam...");
    std::string team_name = PacketDecoder::read_string(packet, offset);
    mcbot::ScoreboardMode mode = (mcbot::ScoreboardMode) PacketDecoder::read_byte(packet, offset);

    log_debug(
        "Team Name: " + team_name +
        "\n\tMode: " + mcbot::to_string(mode));

    // Creating or updating team
    if (mode == mcbot::ScoreboardMode::CREATE || 
        mode == mcbot::ScoreboardMode::UPDATE)
    {
        std::string team_display_name = PacketDecoder::read_string(packet, offset);
        std::string team_prefix = PacketDecoder::read_string(packet, offset);
        std::string team_suffix = PacketDecoder::read_string(packet, offset);

        uint8_t friendly = PacketDecoder::read_byte(packet, offset);
        log_debug("Friendly: " + std::to_string((int)friendly));
        mcbot::FriendlyFire friendly_fire = (mcbot::FriendlyFire) friendly;
        std::string nametag_visibility = PacketDecoder::read_string(packet, offset);
        uint8_t color = PacketDecoder::read_byte(packet, offset);

        log_debug(
            "Team Display Name: " + team_display_name +
            "\n\tTeam Prefix: " + team_prefix +
            "\n\tTeam Suffix: " + team_suffix +
            "\n\tFriendly Fire: " + mcbot::to_string(friendly_fire) +
            "\n\tNametag Visibility: " + nametag_visibility +
            "\n\tColor: " + std::to_string((int)color));
    } 
    
    // Creating or updating players on team
    if (mode == mcbot::ScoreboardMode::CREATE || 
        mode == mcbot::ScoreboardMode::PLAYER_ADDED || 
        mode == mcbot::ScoreboardMode::PLAYER_REMOVED)
    {
        int player_count = PacketDecoder::read_var_int(packet, offset);
        std::list<std::string> players = PacketDecoder::read_string_array(player_count, packet, offset);

        log_debug("Players (" + std::to_string(player_count) + "): ");

        for (auto player : players)
        {
            log_debug("\t\t" + player);
        }
    }

}

void mcbot::MCBot::recv_plugin_message(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutCustomPayload...");

    std::string plugin_channel = PacketDecoder::read_string(packet, offset);
    mcbot::Buffer<uint8_t> data = PacketDecoder::read_byte_array(length - offset, packet, offset);

    log_debug(
        "Plugin Channel: " + plugin_channel +
        "\n\tData: " + data.to_string());
}

void mcbot::MCBot::recv_server_difficulty(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutServerDifficulty...");

    mcbot::Difficulty difficulty = (mcbot::Difficulty) PacketDecoder::read_byte(packet, offset);

    log_debug("Difficulty: " + mcbot::to_string(difficulty));
}

void mcbot::MCBot::recv_world_border(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutWorldBorder...");

    mcbot::WorldBorderAction action = (mcbot::WorldBorderAction) PacketDecoder::read_var_int(packet, offset);

    switch (action)
    {
    case mcbot::WorldBorderAction::SET_SIZE:
    {
        double radius = PacketDecoder::read_double(packet, offset);
        this->world_border.set_radius(radius);
        break;
    }

    case mcbot::WorldBorderAction::LERP_SIZE:
    {
        double old_radius = PacketDecoder::read_double(packet, offset);
        double new_radius = PacketDecoder::read_double(packet, offset);
        long speed = PacketDecoder::read_var_long(packet, offset);
        this->world_border.set_old_radius(old_radius);
        this->world_border.set_new_radius(new_radius);
        this->world_border.set_speed(speed);
        break;
    }

    case mcbot::WorldBorderAction::SET_CENTER:
    {
        double x = PacketDecoder::read_double(packet, offset);
        double z = PacketDecoder::read_double(packet, offset);
        this->world_border.set_x(x);
        this->world_border.set_z(z);
        break;
    }

    case mcbot::WorldBorderAction::INITIALIZE:
    {
        double x = PacketDecoder::read_double(packet, offset);
        double z = PacketDecoder::read_double(packet, offset);
        double old_radius = PacketDecoder::read_double(packet, offset);
        double new_radius = PacketDecoder::read_double(packet, offset);
        long speed = PacketDecoder::read_var_long(packet, offset);
        int portal_teleport_boundary = PacketDecoder::read_var_int(packet, offset);
        int warning_time = PacketDecoder::read_var_int(packet, offset);
        int warning_blocks = PacketDecoder::read_var_int(packet, offset);

        this->world_border.set_x(x);
        this->world_border.set_z(z);
        this->world_border.set_old_radius(old_radius);
        this->world_border.set_new_radius(new_radius);
        this->world_border.set_speed(speed);
        this->world_border.set_portal_teleport_boundary(portal_teleport_boundary);
        this->world_border.set_warning_time(warning_time);
        this->world_border.set_warning_blocks(warning_blocks);
        break;
    }

    case mcbot::WorldBorderAction::SET_WARNING_TIME:
    {
        int warning_time = PacketDecoder::read_var_int(packet, offset);
        this->world_border.set_warning_time(warning_time);
        break;
    }

    case mcbot::WorldBorderAction::SET_WARNING_BLOCKS:
    {
        int warning_blocks = PacketDecoder::read_var_int(packet, offset);
        this->world_border.set_warning_blocks(warning_blocks);
        break;
    }

    }

}

void mcbot::MCBot::recv_title(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutTitle...");

    mcbot::TitleAction action = (mcbot::TitleAction) PacketDecoder::read_var_int(packet, offset);

    switch (action)
    {
    case mcbot::TitleAction::SET_TITLE:
    {
        std::string title_text = PacketDecoder::read_string(packet, offset);
        break;
    }
    case mcbot::TitleAction::SET_SUBTITLE:
    {
        std::string subtitle_text = PacketDecoder::read_string(packet, offset);
        break;
    }
    case mcbot::TitleAction::SET_TIMES_AND_DISPLAY:
    {
        int fade_in = PacketDecoder::read_int(packet, offset);
        int stay = PacketDecoder::read_int(packet, offset);
        int fade_out = PacketDecoder::read_int(packet, offset);
        break;
    }

    }

    log_debug("Action: " + mcbot::to_string(action));
}

void mcbot::MCBot::recv_player_list_header_footer(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("<<< Handling PacketPlayOutPlayerListHeaderFooter...");

    std::string header = PacketDecoder::read_string(packet, offset);
    std::string footer = PacketDecoder::read_string(packet, offset);

    log_debug(
        "Header: " + header +
        "\n\tFooter: " + footer);
}

std::list<mcbot::Entity> mcbot::MCBot::get_entities()
{
    return this->entities;
}

bool mcbot::MCBot::is_connected()
{
    return this->connected;
}

bool mcbot::MCBot::is_ready()
{
    return this->ready;
}

bool mcbot::MCBot::is_encrypted()
{
    return this->encryption_enabled;
}

mcbot::EntityPlayer mcbot::MCBot::get_player()
{
    return this->player;
}

