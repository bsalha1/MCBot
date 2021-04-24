#include "MCBot.h"

#include "StringUtils.h"
#include "JsonObject.h"
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

int32_t mcbot::MCBot::read_var_int(uint8_t* packet, size_t &offset)
{
    int num_read = 0;
    int result = 0;
    uint8_t read;
    do {
        read = packet[offset + num_read];
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

int64_t mcbot::MCBot::read_var_long(uint8_t* packet, size_t& offset)
{
    int num_read = 0;
    long result = 0;
    uint8_t read;
    do {
        read = packet[offset + num_read];
        long value = (read & 0b01111111);
        result |= (value << (7 * num_read));

        num_read++;
        if (num_read > 10)
        {
            offset += num_read;
            fprintf(stderr, "VarLong out of bounds");
        }
    } while ((read & 0b10000000) != 0);

    return result;
}

uint32_t mcbot::MCBot::read_int(uint8_t* packet, size_t& offset)
{
    int result = 
        packet[offset++] << 24 |
        packet[offset++] << 16 | 
        packet[offset++] << 8 | 
        packet[offset++] << 0;
    return result;
}

uint16_t mcbot::MCBot::read_short(uint8_t* packet, size_t& offset)
{
    uint16_t result =
        packet[offset++] << 8 | 
        packet[offset++] << 0;
    return result;
}

uint64_t mcbot::MCBot::read_long(uint8_t* packet, size_t& offset)
{
    uint64_t result =
        ((uint64_t)packet[offset++]) << 56 |
        ((uint64_t)packet[offset++]) << 48 |
        ((uint64_t)packet[offset++]) << 40 |
        ((uint64_t)packet[offset++]) << 32 |
        ((uint64_t)packet[offset++]) << 24 |
        ((uint64_t)packet[offset++]) << 16 |
        ((uint64_t)packet[offset++]) << 8 |
        ((uint64_t)packet[offset++]) << 0;
    return result;
}

uint8_t mcbot::MCBot::read_byte(uint8_t* packet, size_t& offset)
{
    return packet[offset++];
}

float mcbot::MCBot::read_float(uint8_t* packet, size_t& offset)
{
    uint32_t bytes = read_int(packet, offset);
    return *((float*)&bytes);
}

double mcbot::MCBot::read_double(uint8_t* packet, size_t& offset)
{
    uint64_t bytes = read_long(packet, offset);
    return *((double*)&bytes);
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
    int length = read_var_int(packet, offset);
    std::string string = "";
    for (size_t i = 0; i < length; i++)
    {
        string += packet[offset++];
    }
    return string;
}

std::string mcbot::MCBot::read_string(int length, uint8_t* packet, size_t& offset)
{
    std::string string = "";
    for (size_t i = 0; i < length; i++)
    {
        string += packet[offset++];
    }
    return string;
}

mcbot::UUID mcbot::MCBot::read_uuid(uint8_t* packet, size_t& offset)
{
    char bytes[16] = { 0 };
    for (int i = 0; i < 16; i++)
    {
        bytes[i] = packet[offset++];
    }
    return mcbot::UUID(bytes);
}

mcbot::Slot mcbot::MCBot::read_slot(uint8_t* packet, size_t& offset)
{
    bool present = read_boolean(packet, offset);
    if (present)
    {
        int item_id = read_var_int(packet, offset);
        uint8_t item_count = read_byte(packet, offset);
        mcbot::NBT nbt = read_nbt(packet, offset);
        return mcbot::Slot(present, item_id, item_count, nbt);
    }
    else
    {
        return mcbot::Slot(present);
    }
}

void mcbot::MCBot::read_byte_array(uint8_t* bytes, int length, uint8_t* packet, size_t& offset)
{
    for (int i = 0; i < length; i++)
    {
        bytes[i] = packet[offset++];
    }
}

mcbot::Buffer<char> mcbot::MCBot::read_byte_array(int length, uint8_t* packet, size_t &offset)
{
    mcbot::Buffer<char> buffer = mcbot::Buffer<char>(length);
    for (int i = 0; i < length; i++)
    {
        buffer.put(packet[offset++]);
    }
    return buffer;
}

mcbot::Buffer<int> mcbot::MCBot::read_int_array(int length, uint8_t* packet, size_t& offset)
{
    mcbot::Buffer<int> buffer = mcbot::Buffer<int>(length);
    for (int i = 0; i < length; i++)
    {
        buffer.put(read_int(packet, offset));
    }
    return buffer;
}

mcbot::Buffer<long> mcbot::MCBot::read_long_array(int length, uint8_t* packet, size_t& offset)
{
    mcbot::Buffer<long> buffer = mcbot::Buffer<long>(length);
    for (int i = 0; i < length; i++)
    {
        buffer.put(read_long(packet, offset));
    }

    return buffer;
}


std::list<std::string> mcbot::MCBot::read_string_array(int length, uint8_t* packet, size_t& offset)
{
    std::list<std::string> strings;

    for (int i = 0; i < length; i++)
    {
        strings.push_back(read_string(packet, offset));
    }

    return strings;
}

std::list<mcbot::Statistic> mcbot::MCBot::read_statistic_array(int length, uint8_t* packet, size_t& offset)
{
    std::list<mcbot::Statistic> statistics;

    for (int i = 0; i < length; i++)
    {
        std::string name = read_string(packet, offset);
        int value = read_var_int(packet, offset);
        statistics.push_back(mcbot::Statistic(name, value));
    }

    return statistics;
}

std::list<mcbot::PlayerProperty> mcbot::MCBot::read_property_array(int length, uint8_t* packet, size_t& offset)
{
    std::list<mcbot::PlayerProperty> properties;

    for (int i = 0; i < length; i++)
    {
        std::string name = read_string(packet, offset);
        std::string value = read_string(packet, offset);
        bool is_signed = read_boolean(packet, offset);
        std::string signature = is_signed ? read_string(packet, offset) : "";
        properties.push_back(mcbot::PlayerProperty(name, value, is_signed, signature));
    }

    return properties;
}

std::list<mcbot::Slot> mcbot::MCBot::read_slot_array(int length, uint8_t* packet, size_t& offset)
{
    std::list<mcbot::Slot> slots;

    for (int i = 0; i < length; i++)
    {
        bool present = read_boolean(packet, offset);
        if (present)
        {
            int item_id = read_var_int(packet, offset);
            uint8_t item_count = read_byte(packet, offset);
            mcbot::NBT nbt = read_nbt(packet, offset);
            slots.push_back(mcbot::Slot(present, item_id, item_count, nbt));
        }
        else
        {
            slots.push_back(mcbot::Slot(present));
        }
    }

    return slots;
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

std::list<mcbot::NBT> mcbot::MCBot::read_nbt_list(int32_t length, mcbot::NBTType list_type, uint8_t* packet, size_t& offset)
{
    std::list<mcbot::NBT> nbt_list;

    for (int i = 0; i < length; i++)
    {
        nbt_list.push_back(read_nbt(packet, offset, true, list_type));
    }

    return nbt_list;
}

mcbot::NBT mcbot::MCBot::read_nbt(uint8_t* packet, size_t& offset, bool list, mcbot::NBTType list_type)
{
    mcbot::NBTType type;
    mcbot::NBT nbt;

    if (list)
    {
        type = list_type;
    }

    do
    {
        // Lists have no names and contain all the same types 
        std::string name;
        if (!list)
        {
            type = (mcbot::NBTType) read_byte(packet, offset);
            uint16_t name_length = read_short(packet, offset);
            name = read_string(name_length, packet, offset);
        }
        else
        {
            name = "NONE";
        }

        std::cout << "NBT name: " << name << std::endl
            << "NBT type: " << mcbot::to_string(type) << std::endl;

        switch (type)
        {
        case mcbot::NBTType::TAG_END:
        {
            return nbt;
        }

        case mcbot::NBTType::TAG_BYTE:
        {
            nbt.add_byte(name, read_byte(packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_SHORT:
        {
            nbt.add_short(name, read_short(packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_INT:
        {
            nbt.add_int(name, read_int(packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_LONG:
        {
            nbt.add_long(name, read_long(packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_FLOAT:
        {
            nbt.add_float(name, read_float(packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_DOUBLE:
        {
            nbt.add_double(name, read_double(packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_BYTE_ARRAY:
        {
            int32_t length = read_int(packet, offset);
            nbt.add_byte_array(name, read_byte_array(length, packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_INT_ARRAY:
        {
            int32_t length = read_int(packet, offset);
            nbt.add_int_array(name, read_int_array(length, packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_LONG_ARRAY:
        {
            int32_t length = read_int(packet, offset);
            nbt.add_long_array(name, read_long_array(length, packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_STRING:
        {
            uint16_t length = read_short(packet, offset);
            nbt.add_string(name, read_string(length, packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_LIST:
        {
            mcbot::NBTType list_type = (mcbot::NBTType) read_byte(packet, offset);
            int32_t length = read_int(packet, offset);

            if (length == 0 && list_type != NBTType::TAG_END)
            {
                std::cerr << "Found NBT Tag List of size 0 that is not TAG_END" << std::endl;
            }


            nbt.add_nbt_list(name, read_nbt_list(length, list_type, packet, offset));

            break;
        }
        case mcbot::NBTType::TAG_COMPOUND:
        {
            nbt.add_nbt(name, read_nbt(packet, offset, false));
            break;
        }
        }

        if (list)
        {
            return nbt;
        }

    } while (type != mcbot::NBTType::TAG_END);

    return nbt;
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
    //else if (bytes_read > length)
    //{
    //    std::cerr << "Received packet of length " << bytes_read << " vs. " << length << " expected" << std::endl;
        //packet = NULL;
    //    return;
    //}
    log_debug("Received Packet: " + std::to_string(bytes_read) + "bytes");

    return bytes_read;
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

void mcbot::MCBot::update_player_info(mcbot::PlayerInfoAction action, int length, uint8_t* packet, size_t& offset)
{
    for (int i = 0; i < length; i++)
    {
        mcbot::UUID uuid = read_uuid(packet, offset);

        log_debug("\tPlayer Update (" + uuid.to_string() + "): " + mcbot::to_string(action));

        mcbot::Player player;

        switch (action)
        {
        case mcbot::PlayerInfoAction::ADD_PLAYER:
        {
            std::string name = read_string(packet, offset);
            int properties_length = read_var_int(packet, offset);
            std::list<mcbot::PlayerProperty> properties = read_property_array(properties_length, packet, offset);
            mcbot::Gamemode gamemode = (mcbot::Gamemode) read_var_int(packet, offset);
            int ping = read_var_int(packet, offset);
            bool has_display_name = read_boolean(packet, offset);
            std::string display_name = has_display_name ? read_string(packet, offset) : "";

            player = mcbot::Player(uuid, name, properties, gamemode, ping, has_display_name, display_name);
            
            this->uuid_to_player.insert(std::pair<mcbot::UUID, mcbot::Player>(uuid, player));

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
            mcbot::Gamemode gamemode = (mcbot::Gamemode) read_var_int(packet, offset);
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
            int ping = read_var_int(packet, offset);

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

            bool has_display_name = read_boolean(packet, offset);
            std::string display_name = has_display_name ? read_string(packet, offset) : "";
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
    this->encryption_enabled = false;
    this->compression_enabled = false;
    this->uuid_to_player = std::map<mcbot::UUID, mcbot::Player>();
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
    /*if (this->public_key != nullptr)
    {
        free(this->public_key);
    }*/
}

void mcbot::MCBot::log_debug(std::string message)
{
    if (!this->debug)
    {
        return;
    }

    std::cout << "[DEBUG] " << message << std::endl;
}

void mcbot::MCBot::log_error(std::string message)
{
    std::cout << "[ERROR] " << message << std::endl;
}

void mcbot::MCBot::log_info(std::string message)
{
    std::cout << "[INFO] " << message << std::endl;
}

void mcbot::MCBot::log_chat(std::string message)
{
    // TODO
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
    log_debug("Sending PacketHandshakingIn...");
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
        log_error("Failed to send packet");
        print_winsock_error();
    }
}

void mcbot::MCBot::send_login_start()
{
    log_debug("Sending PacketLoginInStart...");
    this->state = mcbot::State::LOGIN;

    uint8_t packet[1028];
    size_t offset = 0;

    write_var_int(0x00, packet, sizeof(packet), offset); // packet id
    write_string_n((char*) this->username.c_str(), packet, sizeof(packet), offset); // username
    write_packet_length(packet, sizeof(packet), offset);

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        log_error("Failed to send packet");
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
    log_debug("Encrypted shared secret with public key");

    unsigned char encrypted_verify_token[256] = { 0 };
    int encrypted_verify_token_len = RSA_public_encrypt(this->verify_token_length, this->verify_token, encrypted_verify_token, rsa_public_key, RSA_PKCS1_PADDING);
    log_debug("Encrypted verify token with public key");

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

void mcbot::MCBot::send_message(char* message)
{
    uint8_t packet[1028];
    size_t offset = 0;

    write_var_int(0x01, packet, sizeof(packet), offset); // packet id

    write_string_n(message, packet, sizeof(packet), offset);

    write_packet_length(packet, sizeof(packet), offset);

    if (this->sock.send_pack(packet, offset) <= 0)
    {
        log_error("Failed to send packet");
        print_winsock_error();
    }
    else
    {
        //log_debug("Sent chat message: " + message)
    }
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
        length -= get_var_int_size(decompressed_length);
        log_debug("Decompressed length: " + std::to_string(decompressed_length));
    }

    int packet_size;
    if (decompressed_length == 0)
    {
        packet = (uint8_t*)calloc(length, sizeof(uint8_t));
        packet_size = read_next_packet(length, packet);
    }
    else
    {
        packet = (uint8_t*)calloc(decompressed_length, sizeof(uint8_t));
        packet_size = read_next_packet(length, packet, decompressed_length);
    }

    size_t offset = 0;
    int packet_id = read_var_int(packet, offset);
    log_debug("ID: " + std::to_string(packet_id));

    handle_recv_packet(packet_id, packet, packet_size, offset);

    free(packet);
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
    else if (this->state == mcbot::State::PLAY)
    {
        switch (packet_id)
        {
            case 0x00:
                this->recv_play_disconnect(packet, length, offset);
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
            case 0x05:
                this->recv_spawn_position(packet, length, offset);
                break;
            case 0x08:
                this->recv_position(packet, length, offset);
                break;
            case 0x09:
                this->recv_held_item_slot(packet, length, offset);
                break;
            case 0x26:
                this->recv_map_chunk_bulk(packet, length, offset);
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
            case 0x41:
                this->recv_server_difficulty(packet, length, offset);
                break;
            case 0x44:
                this->recv_world_border(packet, length, offset);
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
    log_debug("--- Handling PacketLoginOutDisconnect...");

    std::cout << packet << std::endl;
    
    this->connected = false;
    std::cout << "\tDisconnected" << std::endl;

}

void mcbot::MCBot::recv_set_compression(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketLoginOutSetCompression...");
    this->max_uncompressed_length = read_var_int(packet, offset);

    log_debug("\tMax Uncompressed Length: " + std::to_string(this->max_uncompressed_length));
    this->compression_enabled = true;
    this->sock.initialize_compression(this->max_uncompressed_length);
    log_debug("--- COMPRESSION ENABLED ---");
}

void mcbot::MCBot::recv_encryption_request(uint8_t* packet, size_t length, size_t &offset)
{
    log_debug("--- Handling PacketLoginOutEncryptionRequest...");

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
    log_debug("--- Handling PacketPlayLoginOutSuccess...");
    this->state = State::PLAY;

    std::string uuid_string = read_string(packet, offset);
    std::string username = read_string(packet, offset);
    
    log_debug(
        "UUID: " + uuid_string + '\n' +
        "\tUsername: " + username);
}

void mcbot::MCBot::recv_play_disconnect(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutKickDisconnect...");

    std::string reason = read_string(packet, offset);
    this->connected = false;

    log_debug("Reason: " + reason);
}

void mcbot::MCBot::recv_join_server(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutJoinServer...");

    int entity_id = read_int(packet, offset);
    mcbot::Gamemode gamemode = (mcbot::Gamemode) read_byte(packet, offset);
    mcbot::Dimension dimension = (mcbot::Dimension) read_byte(packet, offset);
    mcbot::Difficulty difficulty = (mcbot::Difficulty) read_byte(packet, offset);
    uint8_t max_players = read_byte(packet, offset);
    std::string level_type = read_string(packet, offset);
    bool reduced_debug_info = read_boolean(packet, offset);

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
    log_debug("--- Handling PacketPlayOutChat...");

    std::string chat_data = read_string(packet, offset);
    mcbot::ChatPosition position = (mcbot::ChatPosition) read_byte(packet, offset);

    log_debug("Chat Data: " + chat_data +
        "\n\tPosition: " + mcbot::to_string(position));
}

void mcbot::MCBot::recv_update_time(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutUpdateTime...");

    int64_t world_age = (int64_t) read_long(packet, offset);
    int64_t time_of_day = (int64_t) read_long(packet, offset);

    log_debug("World Age: " + std::to_string(world_age) +
        "\n\tTime of Day: " + std::to_string(time_of_day));
}

void mcbot::MCBot::recv_spawn_position(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutSpawnPosition...");

    mcbot::Location location = read_location(packet, offset);

    log_debug("Location: " + location.to_string());
}

void mcbot::MCBot::recv_position(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutPosition...");

    double x = read_double(packet, offset);
    double y = read_double(packet, offset);
    double z = read_double(packet, offset);
    float yaw = read_float(packet, offset);
    float pitch = read_float(packet, offset);
    uint8_t flags = read_byte(packet, offset);

    log_debug("X: " + std::to_string(x) +
        "\n\tY: " + std::to_string(y) +
        "\n\tZ: " + std::to_string(z) +
        "\n\tYaw: " + std::to_string(yaw) +
        "\n\tPitch: " + std::to_string(pitch) +
        "\n\tFlags: " + std::to_string((int)flags));
}

void mcbot::MCBot::recv_held_item_slot(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutHeldItemSLot...");

    uint8_t held_item_slot = read_byte(packet, offset);

    log_debug("Held Item Slot: " + std::to_string((int) held_item_slot));
}

void mcbot::MCBot::recv_map_chunk_bulk(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutMapChunkBulk...");

    bool sky_light_sent = read_boolean(packet, offset);
    int chunk_column_count = read_var_int(packet, offset);
    int x = read_int(packet, offset);
    int z = read_int(packet, offset);
    unsigned short primary_bit_mask = read_short(packet, offset);

    //log_debug("Loading Chunk (" << x << "," << z << ")" << std::endl;
}

void mcbot::MCBot::recv_game_state_change(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutGameStateChange...");

    uint8_t reason = read_byte(packet, offset);
    float value = read_float(packet, offset);

    log_debug(
        "Reason: " + std::to_string((int)reason) +
        "\n\tValue: " + std::to_string(value));
}

void mcbot::MCBot::recv_set_slot(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutSetSlot...");

    uint8_t window_id = read_byte(packet, offset);
    uint16_t slot_number = read_short(packet, offset);
    mcbot::Slot slot = read_slot(packet, offset);

    log_debug(
        "Window ID: " + std::to_string(window_id) +
        "\n\tSlot: " + std::to_string(slot_number)
    );
}

void mcbot::MCBot::recv_window_items(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutWindowItems...");

    uint8_t window_id = read_byte(packet, offset);
    int16_t count = read_short(packet, offset);
    std::list<mcbot::Slot> slots = read_slot_array(count, packet, offset);

    log_debug(
        "Window ID: " + std::to_string(window_id) +
        "\n\tCount: " + std::to_string(count)
    );
}

void mcbot::MCBot::recv_statistics(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutStatistics...");
    int count = read_var_int(packet, offset);
    std::list<mcbot::Statistic> statistics = read_statistic_array(count, packet, offset);

    log_debug("Statistics (" + std::to_string(count) + "): ");

    for (auto statistic : statistics)
    {
        log_debug("\t" + statistic.to_string());
    }
}

void mcbot::MCBot::recv_player_info(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutPlayerInfo...");

    mcbot::PlayerInfoAction action = (mcbot::PlayerInfoAction) read_var_int(packet, offset);
    int players_length = read_var_int(packet, offset);
    update_player_info(action, players_length, packet, offset);

}

void mcbot::MCBot::recv_abilities(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutAbilities...");
    uint8_t flags = read_byte(packet, offset);
    float flying_speed = read_float(packet, offset);
    float fov_modifier = read_float(packet, offset);

    log_debug(
        "Flags: " + std::to_string((int)flags) +
        "\n\tFlying Speed: " + std::to_string(flying_speed) +
        "\n\tFOV Modifier: " + std::to_string(fov_modifier));
}

void mcbot::MCBot::recv_scoreboard_objective(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutScoreboardObjective...");
    std::string objective_name = read_string(packet, offset);
    mcbot::ScoreboardMode mode = (mcbot::ScoreboardMode) read_byte(packet, offset);

    log_debug(
        "Objective Name: " + objective_name +
        "\n\tMode: " + mcbot::to_string(mode));

    if (mode == mcbot::ScoreboardMode::CREATE ||
        mode == mcbot::ScoreboardMode::UPDATE)
    {
        std::string objective_value = read_string(packet, offset);
        std::string objective_type = read_string(packet, offset);
        log_debug(
            "\tObjective value: " + objective_value +
            "\n\tObjective type: " + objective_type);
    }
}

void mcbot::MCBot::recv_update_scoreboard_score(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutScoreboardScore...");
    std::string score_name = read_string(packet, offset);
    mcbot::ScoreAction action = (mcbot::ScoreAction) read_byte(packet, offset);
    std::string objective_name = read_string(packet, offset);

    log_debug(
        "Score Name: " + score_name +
        "\n\tAction: " + mcbot::to_string(action) +
        "\n\tObjective Name: " + objective_name);

    if (action != mcbot::ScoreAction::REMOVE)
    {
        int value = read_var_int(packet, offset);
        log_debug("\n\tValue: " + std::to_string(value));
    }

}

void mcbot::MCBot::recv_display_scoreboard(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutScoreboardDisplayObjective...");

    mcbot::ScoreboardPosition position = (mcbot::ScoreboardPosition) read_byte(packet, offset);
    std::string score_name = read_string(packet, offset);

    log_debug(
        "Position: " + mcbot::to_string(position) +
        "\n\tScore Name: " + score_name);
}

void mcbot::MCBot::recv_scoreboard_team(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutScoreboardTeam...");
    std::string team_name = read_string(packet, offset);
    mcbot::ScoreboardMode mode = (mcbot::ScoreboardMode) read_byte(packet, offset);

    log_debug(
        "Team Name: " + team_name +
        "\n\tMode: " + mcbot::to_string(mode));

    // Creating or updating team
    if (mode == mcbot::ScoreboardMode::CREATE || 
        mode == mcbot::ScoreboardMode::UPDATE)
    {
        std::string team_display_name = read_string(packet, offset);
        std::string team_prefix = read_string(packet, offset);
        std::string team_suffix = read_string(packet, offset);

        uint8_t friendly = read_byte(packet, offset);
        log_debug("Friendly: " + std::to_string((int)friendly));
        mcbot::FriendlyFire friendly_fire = (mcbot::FriendlyFire) friendly;
        std::string nametag_visibility = read_string(packet, offset);
        uint8_t color = read_byte(packet, offset);

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
        int player_count = read_var_int(packet, offset);
        std::list<std::string> players = read_string_array(player_count, packet, offset);

        log_debug("Players (" + std::to_string(player_count) + "): ");

        for (auto player : players)
        {
            log_debug("\t\t" + player);
        }
    }

}

void mcbot::MCBot::recv_plugin_message(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutCustomPayload...");

    std::string plugin_channel = read_string(packet, offset);
    mcbot::Buffer<char> data = read_byte_array(length - offset, packet, offset);

    log_debug(
        "Plugin Channel: " + plugin_channel +
        "\n\tData: " + data.to_string());
}

void mcbot::MCBot::recv_server_difficulty(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutServerDifficulty...");

    mcbot::Difficulty difficulty = (mcbot::Difficulty) read_byte(packet, offset);

    log_debug("Difficulty: " + mcbot::to_string(difficulty));
}

void mcbot::MCBot::recv_world_border(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutWorldBorder...");

    mcbot::WorldBorderAction action = (mcbot::WorldBorderAction) read_var_int(packet, offset);

    switch (action)
    {
    case mcbot::WorldBorderAction::SET_SIZE:
    {
        double radius = read_double(packet, offset);
        this->world_border.set_radius(radius);
        break;
    }

    case mcbot::WorldBorderAction::LERP_SIZE:
    {
        double old_radius = read_double(packet, offset);
        double new_radius = read_double(packet, offset);
        long speed = read_var_long(packet, offset);
        this->world_border.set_old_radius(old_radius);
        this->world_border.set_new_radius(new_radius);
        this->world_border.set_speed(speed);
        break;
    }

    case mcbot::WorldBorderAction::SET_CENTER:
    {
        double x = read_double(packet, offset);
        double z = read_double(packet, offset);
        this->world_border.set_x(x);
        this->world_border.set_z(z);
        break;
    }

    case mcbot::WorldBorderAction::INITIALIZE:
    {
        double x = read_double(packet, offset);
        double z = read_double(packet, offset);
        double old_radius = read_double(packet, offset);
        double new_radius = read_double(packet, offset);
        long speed = read_var_long(packet, offset);
        int portal_teleport_boundary = read_var_int(packet, offset);
        int warning_time = read_var_int(packet, offset);
        int warning_blocks = read_var_int(packet, offset);

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
        int warning_time = read_var_int(packet, offset);
        this->world_border.set_warning_time(warning_time);
        break;
    }

    case mcbot::WorldBorderAction::SET_WARNING_BLOCKS:
    {
        int warning_blocks = read_var_int(packet, offset);
        this->world_border.set_warning_blocks(warning_blocks);
        break;
    }

    }

}

void mcbot::MCBot::recv_player_list_header_footer(uint8_t* packet, size_t length, size_t& offset)
{
    log_debug("--- Handling PacketPlayOutPlayerListHeaderFooter...");

    std::string header = read_string(packet, offset);
    std::string footer = read_string(packet, offset);

    log_debug(
        "Header: " + header +
        "\n\tFooter: " + footer);
}

bool mcbot::MCBot::is_connected()
{
    return this->connected;
}

bool mcbot::MCBot::is_encrypted()
{
    return this->encryption_enabled;
}

