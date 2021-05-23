#include "PacketReceiver.h"
#include "PacketSender.h"
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

void mcbot::MCBot::update_player_info(mcbot::PlayerInfoAction action, int length, uint8_t* packet, size_t& offset)
{
    for (int i = 0; i < length; i++)
    {
        mcbot::UUID uuid = PacketDecoder::read_uuid(packet, offset);

        log_debug("\tPlayer Update (" + uuid.to_string() + "): " + StringUtils::to_string(action));

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

            if (name == this->session.get_username())
            {
                this->player.set_name(name);
                this->player.set_uuid(uuid);
                this->player.set_properties(properties);
                this->player.set_gamemode(gamemode);
                this->player.set_ping(ping);
                this->player.set_display_name(display_name);
            }
            
            this->register_player(uuid, player);

            log_debug("\t\tName: " + name + '\n'
                + "\t\tGamemode: " + StringUtils::to_string(gamemode) + '\n'
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

mcbot::MCBot::MCBot()
{
    this->debug = false;
    this->connected = false;
    this->ready = false;
    this->uuid_to_player = std::map<mcbot::UUID, mcbot::EntityPlayer>();
    this->world_border = mcbot::WorldBorder();
    this->packet_receiver = new PacketReceiver(this);
    this->packet_sender = new PacketSender(this);

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
    delete this->packet_receiver;
    delete this->packet_sender;
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
        this->log_error("Failed to resolve " + hostname_s);
        print_winsock_error();
        WSACleanup();
        return -1;
    }

    char* address_string = inet_ntoa(((struct sockaddr_in*) result->ai_addr)->sin_addr);
    std::string address_string_s = address_string;

    this->log_info("Resolved " + hostname_s + " to " + address_string_s);

    // Connect //
    this->sock = mcbot::Socket(socket(result->ai_family, result->ai_socktype, result->ai_protocol));
    this->log_info("Connecting to " + address_string_s + ":" + port);
    if (this->sock.connect_socket(result) < 0)
    {
        print_winsock_error();
        WSACleanup();
        return -1;
    }

    this->connected = true;
    this->log_info("Connected to " + address_string_s + ":" + port);

    return 0;
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
            this->packet_sender->send_position(destination, true);
            return;
        }

        try
        {
            this->move(direction);
        }
        catch (const CollisionException & e)
        {
            std::cerr << e.what() << std::endl;
            this->packet_sender->send_position(direction + mcbot::Vector<double>(0, 1, 0), true);
        }

        if (!ignore_ground && !this->on_ground())
        {
            this->move_to_ground(0.10);
        }

        Sleep(1000 / 20);
    }
}

void mcbot::MCBot::move_to(double x, double z, int ticks_per_move)
{
    // Clean up to make math easier
    x = floor(x) + 0.5;
    z = floor(z) + 0.5;

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
            this->packet_sender->send_position(this->player.get_location() + mcbot::Vector<double>(0, 1, 0), false);
            Sleep(1000 / 20 * ticks_per_move);
            this->packet_sender->send_position(this->player.get_location() + mcbot::Vector<double>(abs(dx)/dx, 0, 0) , true);
            Sleep(1000 / 20 * ticks_per_move);
            continue;
        }

        if (!this->on_ground())
        {
            Sleep(1000 / 20 * ticks_per_move);
            this->move_to_ground(0.10);
        }

        Sleep(1000 / 20 * ticks_per_move);
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
            this->packet_sender->send_position(this->player.get_location() + mcbot::Vector<double>(0, 1, 0), false);
            Sleep(1000 / 20 * ticks_per_move);
            this->packet_sender->send_position(this->player.get_location() + mcbot::Vector<double>(0, 0, abs(dz)/dz), true);
            Sleep(1000 / 20 * ticks_per_move);
            continue;
        }

        if (!this->on_ground())
        {
            Sleep(1000 / 20 * ticks_per_move);
            this->move_to_ground(0.10);
        }

        Sleep(1000 / 20 * ticks_per_move);
    }
}

void mcbot::MCBot::attack_entity(Entity entity)
{
    this->packet_sender->send_use_entity(entity.get_id(), UseEntityType::ATTACK);
}

void mcbot::MCBot::register_entity(Entity entity)
{
    this->entities.insert(std::pair<int, Entity>(entity.get_id(), entity));
}

void mcbot::MCBot::remove_entity(int id)
{
    this->entities.erase(id);
}

bool mcbot::MCBot::is_entity_registered(int id)
{
    return this->entities.find(id) != this->entities.end();
}

void mcbot::MCBot::register_player(UUID uuid, EntityPlayer player)
{
    this->uuid_to_player.insert(std::pair<UUID, EntityPlayer>(uuid, player));
}

void mcbot::MCBot::unregister_player(EntityPlayer player)
{
    this->unregister_player(player.get_uuid());
}

void mcbot::MCBot::unregister_player(UUID uuid)
{
    this->uuid_to_player.erase(uuid);
}

mcbot::EntityPlayer& mcbot::MCBot::get_player(UUID uuid)
{
    return this->uuid_to_player[uuid];
}

mcbot::Entity& mcbot::MCBot::get_entity(int id)
{
    return this->entities[id];
}

bool mcbot::MCBot::is_player_registered(UUID uuid)
{
    return this->uuid_to_player.find(uuid) != this->uuid_to_player.end();
}

void mcbot::MCBot::load_chunk(Chunk chunk)
{
    this->chunks.insert(std::pair<std::pair<int, int>, Chunk>(std::pair<int, int>(chunk.get_x(), chunk.get_z()), chunk));
}

mcbot::Chunk& mcbot::MCBot::get_chunk(int x, int z)
{
    return this->chunks[std::pair<int, int>(x, z)];
}

mcbot::Chunk& mcbot::MCBot::get_chunk(mcbot::Vector<int> location)
{
    return this->chunks[std::pair<int, int>(location.get_x() >> 4, location.get_z() >> 4)];
}

mcbot::Chunk& mcbot::MCBot::get_chunk(mcbot::Vector<double> location)
{
    int x = ((int)floor(location.get_x())) >> 4;
    int z = ((int)floor(location.get_z())) >> 4;
    return this->chunks[std::pair<int, int>(x, z)];
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

    Chunk& dest_chunk = this->get_chunk(dest);
    Chunk& init_chunk = this->get_chunk(init);

    int dest_block_id = dest_chunk.get_block_id(dest);

    // Collision detection
    if (dest_block_id != 0 && dest_block_id != 31)
    {
        throw CollisionException(dest_block_id, dest);
    }

    this->packet_sender->send_position(dest, true);
}

void mcbot::MCBot::move(double dx, double dz) 
{
    mcbot::Vector<double> dr = mcbot::Vector<double>(dx, 0, dz);
    mcbot::Vector<double> init = this->player.get_location();
    mcbot::Vector<double> dest = init + dr;

    Chunk& dest_chunk = this->get_chunk(dest);
    Chunk& init_chunk = this->get_chunk(init);
    Block dest_block = Block(dest_chunk.get_block_id(dest));


    // Collision detection
    if (!dest_block.is_weak())
    {
        std::cout << dest_block.get_id() << std::endl;
        throw CollisionException(dest_block, dest);
    }

    float yaw = dx > 0 ? -90 : dx < 0 ? 90 : dz > 0 ? 0 : dz < 0 ? 180 : 0;
    this->packet_sender->send_position_look(dest, yaw, 0, true);
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

void mcbot::MCBot::set_state(State state)
{
    this->state = state;
}

void mcbot::MCBot::set_connected(bool connected)
{
    this->connected = connected;
}

void mcbot::MCBot::set_session(MojangSession session)
{
    this->session = session;
}

mcbot::MojangSession mcbot::MCBot::get_session()
{
    return this->session;
}

std::list<mcbot::Entity> mcbot::MCBot::get_entities()
{
    typedef std::map<int, Entity> map_type;
    std::list<Entity> value_list;
    for (map_type::const_iterator it = this->entities.begin(); it != this->entities.end(); ++it)
    {
        value_list.push_back(it->second);
    }
    return value_list;
}

std::list<mcbot::EntityPlayer> mcbot::MCBot::get_players()
{
    typedef std::map<UUID, EntityPlayer> map_type;
    std::list<EntityPlayer> value_list;
    for (map_type::const_iterator it = this->uuid_to_player.begin(); it != this->uuid_to_player.end(); ++it)
    {
        value_list.push_back(it->second);
    }
    return value_list;
}

bool mcbot::MCBot::is_connected()
{
    return this->connected;
}

bool mcbot::MCBot::is_ready()
{
    return this->ready;
}

mcbot::State mcbot::MCBot::get_state()
{
    return this->state;
}

mcbot::Socket& mcbot::MCBot::get_socket()
{
    return this->sock;
}

mcbot::PacketSender& mcbot::MCBot::get_packet_sender()
{
    return *this->packet_sender;
}

mcbot::PacketReceiver& mcbot::MCBot::get_packet_receiver()
{
    return *this->packet_receiver;
}

mcbot::EntityPlayer& mcbot::MCBot::get_player()
{
    return this->player;
}

