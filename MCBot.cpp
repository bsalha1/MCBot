#include "MCBot.h"
#include "JsonObject.h"
#include "DaftHash.h"
#include "base64.h"


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

int mcbot::MCBot::read_var_int(char* bytes, size_t* offset)
{
    int num_read = 0;
    int result = 0;
    char read;
    int i = 0;
    do {
        read = bytes[(*offset) + num_read];
        int value = (read & 0b01111111);
        result |= (value << (7 * num_read));

        num_read++;
        if (num_read > 5)
        {
            *offset += num_read;
            fprintf(stderr, "VarInt out of bounds");
            return -1;
        }
    } while ((read & 0b10000000) != 0);

    *offset += num_read;
    return result;
}

int mcbot::MCBot::read_string_n(char* string, size_t string_length, char* packet, size_t packet_size, size_t* offset)
{
    int n = read_var_int(packet, offset);
    for (size_t i = 0; i < n && i < string_length; i++)
    {
        string[i] = packet[(*offset)++];
    }
    return n;
}

void mcbot::MCBot::read_byte_array(char* bytes, int bytes_length, char* packet, size_t packet_size, size_t* offset)
{
    for (int i = 0; i < bytes_length; i++)
    {
        bytes[i] = packet[(*offset)++];
    }
}

void mcbot::MCBot::read_byte_array(unsigned char* bytes, int bytes_length, char* packet, size_t packet_size, size_t* offset)
{
    for (int i = 0; i < bytes_length; i++)
    {
        bytes[i] = (unsigned char) packet[(*offset)++];
    }
}

void mcbot::MCBot::write_var_int(int value, char* packet, size_t packet_size, size_t* offset)
{
    do
    {
        char temp = (char)(value & 0b01111111);
        value >>= 7;
        if (value != 0)
        {
            temp |= 0b10000000;
        }
        packet[(*offset)++] = temp;
    } while (value != 0 && (*offset) <= packet_size);
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

void mcbot::MCBot::write_byte_array(unsigned char* bytes, int bytes_length, char* packet, size_t packet_size, size_t* offset)
{
    for (int i = 0; i < bytes_length; i++)
    {
        packet[(*offset)++] = bytes[i];
    }
}

void mcbot::MCBot::write_string_n(char* string, char* packet, size_t packet_size, size_t* offset)
{
    size_t string_length = strlen(string);
    write_var_int(string_length, packet, packet_size, offset);

    for (int i = 0; i < string_length; i++)
    {
        packet[(*offset)++] = string[i];
    }
}

void mcbot::MCBot::write_ushort(unsigned short num, char* packet, size_t packet_size, size_t* offset)
{
    packet[(*offset)++] = num >> 8;
    packet[(*offset)++] = num & 0xFF;
}

void mcbot::MCBot::write_packet_length(char* packet, size_t packet_size, size_t* offset)
{
    int length = *offset;
    int packet_length_size = get_var_int_size(length);

    for (int i = length - 1; i >= 0; i--)
    {
        packet[i + packet_length_size] = packet[i];
    }

    *offset += packet_length_size;
    size_t offset2 = 0;
    write_var_int(length, packet, sizeof(packet), &offset2);
}

mcbot::MCBot::MCBot(std::string email, std::string password)
{
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

int mcbot::MCBot::save_session()
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
    if (getaddrinfo(hostname, port, &hints, &result) < 0)
    {
        std::cout << "Failed to resolve " << hostname << std::endl;
        print_winsock_error();
        WSACleanup();
        return -1;
    }
    char* address_string = inet_ntoa(((struct sockaddr_in*) result->ai_addr)->sin_addr);
    std::cout << "Resolved " << hostname << " to " << address_string << std::endl;

    // Connect //
    this->sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    std::cout << "Connecting to " << address_string << ":" << port << std::endl;
    if (connect(this->sock, result->ai_addr, (int)result->ai_addrlen) < 0)
    {
        print_winsock_error();
        WSACleanup();
        return -1;
    }
    std::cout << "Connected to " << address_string << ":" << port << std::endl;

    return sock;
}

void mcbot::MCBot::send_handshake(char* hostname, unsigned short port)
{
    char packet[1028];
    size_t offset = 0;

    write_var_int(0x00, packet, sizeof(packet), &offset); // packet id
    write_var_int(47, packet, sizeof(packet), &offset);   // protocol version
    write_string_n(hostname, packet, sizeof(packet), &offset); // hostname
    write_ushort(port, packet, sizeof(packet), &offset); // port
    write_var_int(2, packet, sizeof(packet), &offset);   // next state
    write_packet_length(packet, sizeof(packet), &offset);

    if (send(this->sock, packet, offset, 0) <= 0)
    {
        std::cout << "Failed to send packet" << std::endl;
        print_winsock_error();
    }
    else
    {
        std::cout << "Sent handshake" << std::endl;
    }
}

void mcbot::MCBot::send_login_start()
{
    char packet[1028];
    size_t offset = 0;

    write_var_int(0x00, packet, sizeof(packet), &offset); // packet id
    write_string_n((char*) this->username.c_str(), packet, sizeof(packet), &offset); // username
    write_packet_length(packet, sizeof(packet), &offset);

    if (send(this->sock, packet, offset, 0) <= 0)
    {
        std::cout << "Failed to send packet" << std::endl;
        print_winsock_error();
    }
    else
    {
        std::cout << "Sent login start" << std::endl;
    }
}

void mcbot::MCBot::send_encryption_request()
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

    std::cout << "Server Public Key: " << std::endl 
        << pem_formatted << std::endl;



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
    char buffer[1024] = { 0 };
    std::string res = "";
    while (BIO_read(keybio, buffer, 1024) > 0)
    {
        std::cout << buffer;
    }
    BIO_free(keybio);


    unsigned char encrypted_shared_secret[128] = { 0 };
    RSA_public_encrypt(this->shared_secret.length(), (unsigned char*)this->shared_secret.c_str(), encrypted_shared_secret, rsa_public_key, RSA_PKCS1_PADDING);
    std::cout << "Encrypted shared secret with public key" << std::endl;

    unsigned char encrypted_verify_token[128] = { 0 };
    RSA_public_encrypt(this->verify_token_length, this->verify_token, encrypted_verify_token, rsa_public_key, RSA_PKCS1_PADDING);
    std::cout << "Encrypted verify token with public key" << std::endl;

    char packet[1028];
    size_t offset = 0;

    write_var_int(0x01, packet, sizeof(packet), &offset); // packet id

    write_var_int(128, packet, sizeof(packet), &offset); // shared secret length
    write_byte_array(encrypted_shared_secret, 128, packet, sizeof(packet), &offset); // shared secret

    write_var_int(128, packet, sizeof(packet), &offset); // verify token length
    write_byte_array(encrypted_verify_token, 128, packet, sizeof(packet), &offset); // verify token

    write_packet_length(packet, sizeof(packet), &offset);

    if (send(this->sock, packet, offset, 0) <= 0)
    {
        std::cout << "Failed to send packet" << std::endl;
        print_winsock_error();
    }
    else
    {
        std::cout << "Sent encryption response" << std::endl;
    }
}

void mcbot::MCBot::recv_packet()
{
    char packet[1028] = { 0 };
    size_t bytes_read = recv(this->sock, packet, 1028, 0);
    if (bytes_read < 0)
    {
        std::cout << "Failed to receive packet" << std::endl;
        return;
    }
    std::cout << "Received Packet: " << bytes_read << "bytes" << std::endl;

    size_t offset = 0;
    int length = read_var_int(packet, &offset);
    int packet_id = read_var_int(packet, &offset);
    std::cout << "\tLength: " << length << std::endl << "\tPacket ID: " << packet_id << std::endl;
    print_array(packet, bytes_read);
    switch (packet_id)
    {
    case 0x01:
        this->recv_encryption_request(packet, bytes_read, &offset);
        break;
    default:
        std::cout << "Unhandled packet ID: " << packet_id << std::endl;
    }
}

void mcbot::MCBot::recv_encryption_request(char* packet, size_t size_read, size_t* offset)
{
    std::cout << "\tHandling Encryption Request..." << std::endl;

    // Server ID //
    char server_id[64] = { 0 };
    read_string_n(server_id, sizeof(server_id), packet, size_read, offset);
    this->server_id = server_id;

    // Public Key //
    this->public_key_length = read_var_int(packet, offset);
    this->public_key = (const unsigned char*)calloc(public_key_length, sizeof(char));
    read_byte_array((unsigned char *)this->public_key, public_key_length, packet, size_read, offset);

    // Verify Token //
    this->verify_token_length = read_var_int(packet, offset);
    this->verify_token = (const unsigned char*)calloc(verify_token_length, sizeof(char));
    read_byte_array((unsigned char*)this->verify_token, verify_token_length, packet, size_read, offset);
}
