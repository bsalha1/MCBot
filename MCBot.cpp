#include "MCBot.h"
#include "JsonObject.h"
#include "DaftHash.h"
#include "base64.h"

static float unpack_float(uint8_t* buf)
{
    uint32_t temp = 0;
    temp = 
        (buf[0] << 24) |
        (buf[1] << 16) |
        (buf[2] << 8) |
        (buf[3] << 0);
    return *((float*)&temp);
}

static void print_array(char* arr, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        printf("%d ", (unsigned char)arr[i]);
    }
    printf("\n");
}

static void print_array(unsigned char* arr, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

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

int mcbot::MCBot::read_var_int(uint8_t* bytes, size_t &offset)
{
    int num_read = 0;
    int result = 0;
    char read;
    int i = 0;
    do {
        read = bytes[offset + num_read];
        int value = (read & 0b01111111);
        result |= (value << (7 * num_read));

        num_read++;
        if (num_read > 5)
        {
            offset += num_read;
            fprintf(stderr, "VarInt out of bounds");
            return -1;
        }
    } while ((read & 0b10000000) != 0);

    offset += num_read;
    return result;
}

int mcbot::MCBot::read_int(uint8_t* packet, size_t& offset)
{
    uint8_t byte4 = packet[offset++];
    uint8_t byte3 = packet[offset++];
    uint8_t byte2 = packet[offset++];
    uint8_t byte1 = packet[offset++];

    int result = byte4 << 24 | byte3 << 16 | byte2 << 8 | byte1 << 0;
    return result;
}

uint16_t mcbot::MCBot::read_ushort(uint8_t* packet, size_t& offset)
{
    uint8_t byte2 = packet[offset++];
    uint8_t byte1 = packet[offset++];

    unsigned short result = byte2 << 8 | byte1 << 0;
    return result;
}

int8_t mcbot::MCBot::read_byte(uint8_t* packet, size_t& offset)
{
    return packet[offset++];
}

uint8_t mcbot::MCBot::read_ubyte(uint8_t* packet, size_t& offset)
{
    return packet[offset++];
}

float mcbot::MCBot::read_float(uint8_t* packet, size_t& offset)
{
    uint32_t bytes = read_int(packet, offset);
    return *((float*)&bytes);
}

bool mcbot::MCBot::read_boolean(uint8_t* packet, size_t& offset)
{
    uint8_t value = packet[offset++];
    bool result = false;
    if (value == 1)
    {
        result = true;
    }
    else if (value == 0)
    {
        result = false;
    }
    else
    {
        std::cerr << "Invalid boolean value read: " << value << std::endl;
    }

    return result;
}

std::string mcbot::MCBot::read_string(uint8_t* packet, size_t &offset)
{
    int n = read_var_int(packet, offset);
    std::string string = "";
    for (size_t i = 0; i < n; i++)
    {
        string += packet[offset++];
    }
    return string;
}

void mcbot::MCBot::read_byte_array(uint8_t* bytes, int bytes_length, uint8_t* packet, size_t &offset)
{
    for (int i = 0; i < bytes_length; i++)
    {
        bytes[i] = packet[offset++];
    }
}


std::list<std::string> mcbot::MCBot::read_string_array(int string_length, uint8_t* packet, size_t& offset)
{
    std::list<std::string> strings;

    for (int i = 0; i < string_length; i++)
    {
        strings.push_back(read_string(packet, offset));
    }

    return strings;
}

std::list<mcbot::Statistic> mcbot::MCBot::read_statistic_array(int statistics_length, uint8_t* packet, size_t& offset)
{
    std::list<mcbot::Statistic> statistics;

    for (int i = 0; i < statistics_length; i++)
    {
        std::string name = read_string(packet, offset);
        int value = read_var_int(packet, offset);
        statistics.push_back(mcbot::Statistic(name, value));
    }

    return statistics;
}

mcbot::Location mcbot::MCBot::read_location(uint8_t* packet, size_t& offset)
{
    // Parse X Coordinate (26 bit signed integer)
    uint8_t byte4 = packet[offset++];
    uint8_t byte3 = packet[offset++];
    uint8_t byte2 = packet[offset++];
    uint8_t byte1 = packet[offset] & 0xC0; // xx00 0000

    uint32_t x_bits = ((byte4 << 24) | (byte3 << 16) | (byte2 << 8) | (byte1 << 0)) >> 6;
    int32_t x = x_bits & (1 << 25) ? x_bits - (0x3FFFFFF + 1) : x_bits;

    // Parse Y Coordinate (12 bit signed integer)
    byte2 = packet[offset++] & 0x3F;
    byte1 = packet[offset] & 0xFC;

    uint16_t y_bits = ((byte2 << 8) | (byte1 << 0)) >> 2;
    int16_t y = y_bits & (1 << 11) ? y_bits - (0xFFF + 1) : y_bits;

    // Parse Z Coordinate (26 bit signed integer)
    byte4 = packet[offset++] & 0x03;
    byte3 = packet[offset++];
    byte2 = packet[offset++];
    byte1 = packet[offset++];

    uint32_t z_bits = (byte4 << 24) | (byte3 << 16) | (byte2 << 8) | (byte1 << 0);
    int32_t z = z_bits & (1 << 25) ? z_bits - (0x3FFFFFF + 1) : z_bits;

    return mcbot::Location(x, y, z);
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

void mcbot::MCBot::read_next_packet(int length, uint8_t* packet)
{
    size_t bytes_read = this->sock.recv_packet(packet, length);
    if (bytes_read < 0)
    {
        std::cerr << "Failed to receive packet" << std::endl;
        packet = NULL;
        return;
    }
    else if (bytes_read != length)
    {
        std::cerr << "Received packet of length " << bytes_read << " vs. " << length << " expected" << std::endl;
        packet = NULL;
        return;
    }
    std::cout << "Received Packet: " << bytes_read << "bytes" << std::endl;
}


void mcbot::MCBot::write_var_int(int value, uint8_t* packet, size_t packet_size, size_t &offset)
{
    do
    {
        char temp = (char)(value & 0b01111111);
        value >>= 7;
        if (value != 0)
        {
            temp |= 0b10000000;
        }
        packet[offset++] = temp;
    } while (value != 0 && offset <= packet_size);
}

size_t mcbot::MCBot::get_var_int_size(int value)
{
    size_t size = 0;
    do
    {
        char temp = (char)(value & 0b01111111);
        value >>= 7;
        if (value != 0)
        {
            temp |= 0b10000000;
        }
        size++;
    } while (value != 0);
    return size;
}

void mcbot::MCBot::write_byte_array(uint8_t* bytes, int bytes_length, uint8_t* packet, size_t packet_size, size_t &offset)
{
    for (int i = 0; i < bytes_length; i++)
    {
        packet[offset++] = bytes[i];
    }
}

void mcbot::MCBot::write_string_n(char* string, uint8_t* packet, size_t packet_size, size_t &offset)
{
    size_t string_length = strlen(string);
    write_var_int(string_length, packet, packet_size, offset);

    for (int i = 0; i < string_length; i++)
    {
        packet[offset++] = string[i];
    }
}

void mcbot::MCBot::write_ushort(unsigned short num, uint8_t* packet, size_t packet_size, size_t &offset)
{
    packet[offset++] = num >> 8;
    packet[offset++] = num & 0xFF;
}

void mcbot::MCBot::write_packet_length(uint8_t* packet, size_t packet_size, size_t &offset)
{
    int length = offset;
    int packet_length_size = get_var_int_size(length);

    for (int i = length - 1; i >= 0; i--)
    {
        packet[i + packet_length_size] = packet[i];
    }

    offset += packet_length_size;
    size_t offset2 = 0;
    write_var_int(length, packet, sizeof(packet), offset2);
}

mcbot::MCBot::MCBot(std::string email, std::string password)
{
    this->connected = false;
    this->encryption_enabled = false;
    this->compression_enabled = false;
    this->email = email;
    this->password = password;

    // Start WinSock DLL //
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout << "Failed to start up WinSock DLL" << std::endl;
        print_winsock_error();
        return;
    }
    std::cout << "WinSock DLL Started" << std::endl;
}

mcbot::MCBot::~MCBot()
{
    /*if (this->public_key != nullptr)
    {
        free(this->public_key);
    }*/
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
    auto response_json = mcbot::JsonObject::serialize(response->body);
    this->access_token = response_json.get_string("accessToken");
    
    auto selected_profile = response_json.get_object("selectedProfile");
    this->username = selected_profile.get_string("name");
    this->uuid = selected_profile.get_string("id");

    std::cout << "Logged into Mojang account:" << std::endl
        << "\tAccount resolved to " << this->username << std::endl;

    return 0;
}

int mcbot::MCBot::verify_access_token()
{
    // Payload //
    char content_format[] = 
        "{"
            "\"accessToken\" : \"%s\""
        "}\r\n";
    char content[1024] = { 0 };
    sprintf_s(content, content_format, this->access_token.c_str());

    // Send Payload //
    httplib::Client cli("https://authserver.mojang.com");
    auto response = cli.Post("/validate", content, "application/json");

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
    char content_format[] =
        "{"
            "\"accessToken\" : \"%s\","
            "\"selectedProfile\": \"%s\","
            "\"serverId\": \"%s\""
        "}\r\n";
    char content[1024] = { 0 };
    sprintf_s(content, content_format, this->access_token.c_str(), this->uuid.c_str(), hash.c_str());

    // Send Payload //
    httplib::Client cli("https://sessionserver.mojang.com");
    auto response = cli.Post("/session/minecraft/join", content, "application/json");

    // If response is 204 then session is valid
    return response->status == 204 ? 0 : -1;
}

int mcbot::MCBot::connect_server(char* hostname, char* port)
{
    // Resolve Host //
    struct addrinfo hints;
    struct addrinfo* result;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if (getaddrinfo(hostname, port, &hints, &result) < 0 || result == NULL)
    {
        std::cout << "Failed to resolve " << hostname << std::endl;
        print_winsock_error();
        WSACleanup();
        return -1;
    }

    char* address_string = inet_ntoa(((struct sockaddr_in*) result->ai_addr)->sin_addr);
    std::cout << "Resolved " << hostname << " to " << address_string << std::endl;

    // Connect //
    this->sock = mcbot::Socket(socket(result->ai_family, result->ai_socktype, result->ai_protocol));
    std::cout << "Connecting to " << address_string << ":" << port << std::endl;
    if (this->sock.connect_socket(result) < 0)
    {
        print_winsock_error();
        WSACleanup();
        return -1;
    }

    this->connected = true;
    std::cout << "Connected to " << address_string << ":" << port << std::endl;

    return 0;
}

void mcbot::MCBot::send_handshake(char* hostname, unsigned short port)
{
    std::cout << "Sending PacketHandshakingIn..." << std::endl;
    this->state = mcbot::State::HANDSHAKE;

    uint8_t packet[1028];
    size_t offset = 0;

    write_var_int(0x00, packet, sizeof(packet), offset); // packet id
    write_var_int(47, packet, sizeof(packet), offset);   // protocol version
    write_string_n(hostname, packet, sizeof(packet), offset); // hostname
    write_ushort(port, packet, sizeof(packet), offset); // port
    write_var_int(2, packet, sizeof(packet), offset);   // next state
    write_packet_length(packet, sizeof(packet), offset);

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        std::cout << "Failed to send packet" << std::endl;
        print_winsock_error();
    }
}

void mcbot::MCBot::send_login_start()
{
    std::cout << "Sending PacketLoginInStart..." << std::endl;
    this->state = mcbot::State::LOGIN;

    uint8_t packet[1028];
    size_t offset = 0;

    write_var_int(0x00, packet, sizeof(packet), offset); // packet id
    write_string_n((char*) this->username.c_str(), packet, sizeof(packet), offset); // username
    write_packet_length(packet, sizeof(packet), offset);

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        std::cout << "Failed to send packet" << std::endl;
        print_winsock_error();
    }
}

void mcbot::MCBot::send_encryption_response()
{
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
    char* pem_formatted = (char*) malloc(pem_formatted_size);
    sprintf_s(pem_formatted, pem_formatted_size, pem_format, temp.c_str());

    // Get RSA corresponding to PEM
    BIO* bio = BIO_new_mem_buf((void*)pem_formatted, -1);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    RSA* rsa_public_key = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
    if (rsa_public_key == NULL)
    {
        printf("ERROR: Could not load PUBLIC KEY! PEM_read_bio_RSA_PUBKEY FAILED: %s\n", ERR_error_string(ERR_get_error(), NULL));
    }
    BIO_free(bio);
    free(pem_formatted);

    std::cout << "Public Key Info: " << std::endl;
    BIO* keybio = BIO_new(BIO_s_mem());
    RSA_print(keybio, rsa_public_key, 0);
    char buffer[2048] = { 0 };
    std::string res = "";
    while (BIO_read(keybio, buffer, 2048) > 0)
    {
        std::cout << buffer;
    }
    BIO_free(keybio);


    unsigned char encrypted_shared_secret[256] = { 0 };
    int encrypted_shared_secret_len = RSA_public_encrypt(this->shared_secret.length(), (unsigned char*)this->shared_secret.c_str(), encrypted_shared_secret, rsa_public_key, RSA_PKCS1_PADDING);
    std::cout << "Encrypted shared secret with public key" << std::endl;

    unsigned char encrypted_verify_token[256] = { 0 };
    int encrypted_verify_token_len = RSA_public_encrypt(this->verify_token_length, this->verify_token, encrypted_verify_token, rsa_public_key, RSA_PKCS1_PADDING);
    std::cout << "Encrypted verify token with public key" << std::endl;

    uint8_t packet[1028];
    size_t offset = 0;

    write_var_int(0x01, packet, sizeof(packet), offset); // packet id

    write_var_int(encrypted_shared_secret_len, packet, sizeof(packet), offset); // shared secret length
    write_byte_array(encrypted_shared_secret, encrypted_shared_secret_len, packet, sizeof(packet), offset); // shared secret

    write_var_int(encrypted_verify_token_len, packet, sizeof(packet), offset); // verify token length
    write_byte_array(encrypted_verify_token, encrypted_verify_token_len, packet, sizeof(packet), offset); // verify token

    write_packet_length(packet, sizeof(packet), offset);

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        std::cout << "Failed to send packet" << std::endl;
        print_winsock_error();
    }
    else
    {
        this->encryption_enabled = true;
        this->sock.initialize_encryption((unsigned char*)this->shared_secret.c_str(), (unsigned char*)this->shared_secret.c_str());
        std::cout << "Sent encryption response - ENCRYPTION ENABLED" << std::endl;
    }
}

void mcbot::MCBot::send_message(char* message)
{
    uint8_t packet[1028];
    size_t offset = 0;

    write_var_int(0x01, packet, sizeof(packet), offset); // packet id

    write_string_n(message, packet, sizeof(packet), offset);

    write_packet_length(packet, sizeof(packet), offset);

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        std::cout << "Failed to send packet" << std::endl;
        print_winsock_error();
    }
    else
    {
        std::cout << "Sent chat message: " << message << std::endl;
    }
}


void mcbot::MCBot::handle_recv_packet(int packet_id, uint8_t* packet, int length, size_t &offset)
{
    if (this->state == mcbot::State::HANDSHAKE)
    {

    }
    else if (this->state == mcbot::State::STATUS)
    {

    }
    else if (this->state == mcbot::State::LOGIN)
    {
        switch (packet_id)
        {
            case 0x00:
                this->recv_disconnect(packet, length, offset);
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
                std::cerr << "UNHANDLED " << this->state << " PACKET ID: " << packet_id << std::endl;
        }
    }
    else if (this->state == mcbot::State::PLAY)
    {
        switch (packet_id)
        {
            case 0x01:
                this->recv_join_server(packet, length, offset);
                break;
            case 0x05:
                this->recv_spawn_position(packet, length, offset);
                break;
            case 0x09:
                this->recv_held_item_slot(packet, length, offset);
                break;
            case 0x26:
                this->recv_map_chunk_bulk(packet, length, offset);
                break;
            case 0x37:
                this->recv_statistics(packet, length, offset);
                break;
            case 0x39:
                this->recv_abilities(packet, length, offset);
                break;
            case 0x3E:
                this->recv_scoreboard_team(packet, length, offset);
                break;
            case 0x3F:
                this->recv_plugin_message(packet, length, offset);
                break;
            case 0x41:
                this->recv_server_difficulty(packet, length, offset);
                break;
            default:
                std::cerr << "UNHANDLED " << this->state << " PACKET ID: " << packet_id << std::endl;
        }
    }
    else
    {
        std::cerr << "UNKNOWN STATE: " << this->state << std::endl;
    }
}


void mcbot::MCBot::recv_packet()
{
    int length = this->read_next_var_int();
    std::cout << "Length: " << length << std::endl;

    uint8_t* packet = (uint8_t*) calloc(length, sizeof(uint8_t));
    read_next_packet(length, packet);

    size_t offset = 0;
    if (this->compression_enabled)
    {
        int uncompressed_length = read_var_int(packet, offset);
        std::cout << "Uncompressed Length: " << uncompressed_length << std::endl;
    }
    int packet_id = read_var_int(packet, offset);
    std::cout << "ID: " << packet_id << std::endl;

    handle_recv_packet(packet_id, packet, length, offset);


    free(packet);
}

void mcbot::MCBot::recv_disconnect(uint8_t* packet, size_t length, size_t& offset)
{
    std::cout << "--- Handling PacketLoginOutDisconnect..." << std::endl;

    std::cout << packet << std::endl;
    
    this->connected = false;
    std::cout << "\tDisconnected" << std::endl;

}

void mcbot::MCBot::recv_set_compression(uint8_t* packet, size_t length, size_t& offset)
{
    std::cout << "--- Handling PacketLoginOutSetCompression..." << std::endl;
    this->max_uncompressed_length = read_var_int(packet, offset);

    std::cout << "\tMax Uncompressed Length: " << this->max_uncompressed_length << std::endl;
    this->compression_enabled = true;
    this->sock.initialize_compression(this->max_uncompressed_length);
    std::cout << "--- COMPRESSION ENABLED ---" << std::endl;
}

void mcbot::MCBot::recv_encryption_request(uint8_t* packet, size_t length, size_t &offset)
{
    std::cout << "--- Handling PacketLoginOutEncryptionRequest..." << std::endl;

    // Server ID //
    this->server_id = read_string(packet, offset);

    // Public Key //
    this->public_key_length = read_var_int(packet, offset);
    this->public_key = (uint8_t*)calloc(public_key_length, sizeof(uint8_t));
    read_byte_array(this->public_key, public_key_length, packet, offset);

    // Verify Token //
    this->verify_token_length = read_var_int(packet, offset);
    this->verify_token = (uint8_t*)calloc(verify_token_length, sizeof(uint8_t));
    read_byte_array(this->verify_token, verify_token_length, packet, offset);

    // Save Session //
    // - So Yggdrasil authentication doesn't kick us!
    std::cout << "Saving session..." << std::endl;
    if (this->send_session() < 0)
    {
        std::cerr << "Invalid session!" << std::endl;
    }
    else
    {
        this->send_encryption_response();
    }
}

void mcbot::MCBot::recv_login_success(uint8_t* packet, size_t length, size_t& offset)
{
    std::cout << "--- Handling PacketPlayLoginOutSuccess..." << std::endl;
    this->state = State::PLAY;

    std::string uuid_string = read_string(packet, offset);
    std::string username = read_string(packet, offset);
    
    std::cout << "\tUUID: " << uuid_string << std::endl
        << "\tUsername: " << username << std::endl;
}

void mcbot::MCBot::recv_join_server(uint8_t* packet, size_t length, size_t& offset)
{
    std::cout << "--- Handling PacketPlayOutJoinServer..." << std::endl;

    int entity_id = read_int(packet, offset);
    mcbot::Gamemode gamemode = (mcbot::Gamemode) read_ubyte(packet, offset);
    mcbot::Dimension dimension = (mcbot::Dimension) read_byte(packet, offset);
    mcbot::Difficulty difficulty = (mcbot::Difficulty) read_ubyte(packet, offset);
    uint8_t max_players = read_ubyte(packet, offset);
    std::string level_type = read_string(packet, offset);
    bool reduced_debug_info = read_boolean(packet, offset);

    std::cout << "\tEntity ID: " << entity_id << std::endl
        << "\tGamemode: " << gamemode << std::endl
        << "\tDimension: " << dimension << std::endl
        << "\tDifficulty: " << difficulty << std::endl
        << "\tMax Players: " << (int)max_players << std::endl
        << "\tLevel Type: " << level_type << std::endl
        << "\tReduced Debug Info: " << reduced_debug_info << std::endl;
}

void mcbot::MCBot::recv_spawn_position(uint8_t* packet, size_t length, size_t& offset)
{
    std::cout << "--- Handling PacketPlayOutSpawnPosition..." << std::endl;

    mcbot::Location location = read_location(packet, offset);

    std::cout << "\tLocation: " << location << std::endl;
}

void mcbot::MCBot::recv_held_item_slot(uint8_t* packet, size_t length, size_t& offset)
{
    std::cout << "--- Handling PacketPlayOutHeldItemSLot..." << std::endl;

    uint8_t held_item_slot = read_byte(packet, offset);

    std::cout << "\tHeld Item Slot: " << (int) held_item_slot << std::endl;
}

void mcbot::MCBot::recv_map_chunk_bulk(uint8_t* packet, size_t length, size_t& offset)
{
    std::cout << "--- Handling PacketPlayOutMapChunkBulk..." << std::endl;

    bool sky_light_sent = read_boolean(packet, offset);
    int chunk_column_count = read_var_int(packet, offset);
    int x = read_int(packet, offset);
    int z = read_int(packet, offset);
    unsigned short primary_bit_mask = read_ushort(packet, offset);

    std::cout << "Loading Chunk (" << x << "," << z << ")" << std::endl;
}

void mcbot::MCBot::recv_statistics(uint8_t* packet, size_t length, size_t& offset)
{
    std::cout << "--- Handling PacketPlayOutStatistics..." << std::endl;
    int count = read_var_int(packet, offset);
    std::list<mcbot::Statistic> statistics = read_statistic_array(count, packet, offset);

    std::cout << "\tStatistics (" << count << "): " << std::endl;

    for (auto statistic : statistics)
    {
        std::cout << "\t\t" << statistic << std::endl;
    }
}

void mcbot::MCBot::recv_abilities(uint8_t* packet, size_t length, size_t& offset)
{
    std::cout << "--- Handling PacketPlayOutAbilities..." << std::endl;
    uint8_t flags = read_ubyte(packet, offset);
    float flying_speed = read_float(packet, offset);
    float fov_modifier = read_float(packet, offset);

    std::cout << "\tFlags: " << flags << std::endl
        << "\tFlying Speed: " << flying_speed << std::endl
        << "\tFOV Modifier: " << fov_modifier << std::endl;
}

void mcbot::MCBot::recv_scoreboard_team(uint8_t* packet, size_t length, size_t& offset)
{
    std::cout << "--- Handling PacketPlayOutScoreboardTeam..." << std::endl;
    std::string team_name = read_string(packet, offset);
    uint8_t mode = read_ubyte(packet, offset);

    std::cout << "\tTeam Name: " << team_name << std::endl
        << "\tMode: " << (int) mode << std::endl;

    if (mode == 0 || mode == 2)
    {
        std::string team_display_name = read_string(packet, offset);
        std::string team_prefix = read_string(packet, offset);
        std::string team_suffix = read_string(packet, offset);
        mcbot::FriendlyFire friendly_fire = (mcbot::FriendlyFire) read_ubyte(packet, offset);
        std::string nametag_visibility = read_string(packet, offset);
        uint8_t color = read_ubyte(packet, offset);

        std::cout << "\tTeam Display Name: " << team_display_name << std::endl
            << "\tTeam Prefix: " << team_prefix << std::endl
            << "\tTeam Suffix: " << team_suffix << std::endl
            << "\tFriendly Fire: " << friendly_fire << std::endl
            << "\tNametag Visibility: " << nametag_visibility << std::endl
            << "\tColor: " << (int)color << std::endl;
    } 
    
    if (mode == 0 || mode == 3 || mode == 4)
    {
        int player_count = read_var_int(packet, offset);
        std::list<std::string> players = read_string_array(player_count, packet, offset);

        std::cout << "\tPlayers (" << player_count << "): " << std::endl;

        for (auto player : players)
        {
            std::cout << "\t\t" << player << std::endl;
        }
    }

}

void mcbot::MCBot::recv_plugin_message(uint8_t* packet, size_t length, size_t& offset)
{
    std::cout << "--- Handling PacketPlayOutCustomPayload..." << std::endl;

    std::string plugin_channel = read_string(packet, offset);

    uint8_t* data = (uint8_t*)calloc(length - offset, sizeof(uint8_t));
    read_byte_array(data, length - offset, packet, offset);

    std::cout << "\tPlugin Channel: " << plugin_channel << std::endl
        << "\tData: " << data << std::endl;

    free(data);
}

void mcbot::MCBot::recv_server_difficulty(uint8_t* packet, size_t length, size_t& offset)
{
    std::cout << "--- Handling PacketPlayOutServerDifficulty..." << std::endl;

    mcbot::Difficulty difficulty = (mcbot::Difficulty) read_ubyte(packet, offset);

    std::cout << "\tDifficulty: " << difficulty << std::endl;
}

bool mcbot::MCBot::is_connected()
{
    return this->connected;
}

bool mcbot::MCBot::is_encrypted()
{
    return this->encryption_enabled;
}

