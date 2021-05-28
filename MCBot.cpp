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

    void MCBot::UpdatePlayerInfo(PlayerInfoAction action, int length, uint8_t* packet, size_t& offset)
    {
        for (int i = 0; i < length; i++)
        {
            UUID uuid = PacketDecoder::ReadUUID(packet, offset);

            LogDebug("\tPlayer Update (" + uuid.ToString() + "): " + StringUtils::to_string(action));

            EntityPlayer player;

            switch (action)
            {
            case PlayerInfoAction::ADD_PLAYER:
            {
                std::string name = PacketDecoder::ReadString(packet, offset);
                int properties_length = PacketDecoder::ReadVarInt(packet, offset);
                std::list<PlayerProperty> properties = PacketDecoder::ReadPropertyArray(properties_length, packet, offset);
                Gamemode gamemode = (Gamemode) PacketDecoder::ReadVarInt(packet, offset);
                int ping = PacketDecoder::ReadVarInt(packet, offset);
                bool has_display_name = PacketDecoder::ReadBoolean(packet, offset);
                std::string display_name = has_display_name ? PacketDecoder::ReadString(packet, offset) : name;

                player = EntityPlayer(-1, uuid, name, properties, gamemode, ping, display_name);

                if (name == this->session.GetUsername())
                {
                    this->player.SetName(name);
                    this->player.SetUUID(uuid);
                    this->player.SetProperties(properties);
                    this->player.SetGamemode(gamemode);
                    this->player.SetPing(ping);
                    this->player.SetDisplayName(display_name);
                }

                this->RegisterPlayer(uuid, player);

                LogDebug("\t\tName: " + name + '\n'
                    + "\t\tGamemode: " + StringUtils::to_string(gamemode) + '\n'
                    + "\t\tPing: " + std::to_string(ping));
                break;
            }

            case PlayerInfoAction::UPDATE_GAMEMODE:
            {
                try
                {
                    player = this->uuid_to_player.at(uuid);
                }
                catch (...)
                {
                    LogError("Failed to find player of UUID " + uuid.ToString());
                    return;
                }
                Gamemode gamemode = (Gamemode) PacketDecoder::ReadVarInt(packet, offset);
                player.SetGamemode(gamemode);
                break;
            }

            case PlayerInfoAction::UPDATE_LATENCY:
            {
                try
                {
                    player = this->uuid_to_player.at(uuid);
                }
                catch (...)
                {
                    LogError("Failed to find player of UUID " + uuid.ToString());
                    return;
                }
                int ping = PacketDecoder::ReadVarInt(packet, offset);

                player.SetPing(ping);
                break;
            }

            case PlayerInfoAction::UPDATE_DISPLAY_NAME:
            {
                try
                {
                    player = this->uuid_to_player.at(uuid);
                }
                catch (...)
                {
                    LogError("Failed to find player of UUID " + uuid.ToString());
                    return;
                }

                bool has_display_name = PacketDecoder::ReadBoolean(packet, offset);
                std::string display_name = has_display_name ? PacketDecoder::ReadString(packet, offset) : "";
                if (has_display_name)
                {
                    player.SetDisplayName(display_name);
                }

                break;
            }

            case PlayerInfoAction::REMOVE_PLAYER:
            {
                this->uuid_to_player.erase(uuid);
                break;
            }
            default:
                break;
            }
        }
    }

    MCBot::MCBot()
    {
        this->debug = false;
        this->connected = false;
        this->ready = false;
        this->uuid_to_player = std::map<UUID, EntityPlayer>();
        this->world_border = WorldBorder();
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

    MCBot::~MCBot()
    {
        delete this->packet_receiver;
        delete this->packet_sender;
    }

    int MCBot::ConnectToServer(char* hostname, char* port)
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
            this->LogError("Failed to resolve " + hostname_s);
            print_winsock_error();
            WSACleanup();
            return -1;
        }

        char* address_string = inet_ntoa(((struct sockaddr_in*) result->ai_addr)->sin_addr);
        std::string address_string_s = address_string;

        this->LogInfo("Resolved " + hostname_s + " to " + address_string_s);

        // Connect //
        this->sock = Socket(socket(result->ai_family, result->ai_socktype, result->ai_protocol));
        this->LogInfo("Connecting to " + address_string_s + ":" + port);
        if (this->sock.connect_socket(result) < 0)
        {
            print_winsock_error();
            WSACleanup();
            return -1;
        }

        this->connected = true;
        this->LogInfo("Connected to " + address_string_s + ":" + port);

        return 0;
    }

    void MCBot::MoveTo(Vector<double> destination, double speed, bool ignore_ground)
    {
        while (this->player.GetLocation().Distance(destination) >= 0)
        {
            Vector<double> current_location = this->player.GetLocation();
            Vector<double> direction = destination - current_location;

            direction.Scale(1 / direction.Magnitude());
            direction.Scale(speed);

            // If we are close enough, just move us to the location instantly
            if (this->player.GetLocation().Distance(destination) <= 1.0)
            {
                this->packet_sender->SendPosition(destination, true);
                return;
            }

            try
            {
                this->Move(direction);
            }
            catch (const CollisionException & e)
            {
                std::cerr << e.what() << std::endl;
                this->packet_sender->SendPosition(direction + Vector<double>(0, 1, 0), true);
            }

            if (!ignore_ground && !this->OnGround())
            {
                this->MoveToGround(0.10);
            }

            Sleep(1000 / 20);
        }
    }

    void MCBot::MoveTo(double x, double z, int ticks_per_move)
    {
        // Clean up to make math easier
        x = floor(x) + 0.5;
        z = floor(z) + 0.5;

        Vector<double> dest = Vector<double>(x, 0, z);
        Vector<double> init = this->player.GetLocation();

        auto diff = dest - init;

        double dx = abs(diff.GetX()) / diff.GetX();
        double dz = abs(diff.GetZ()) / diff.GetZ();

        for (double x = init.GetX(); x != dest.GetX(); x += dx)
        {
            Vector<double> current_location = this->player.GetLocation();

            try
            {
                this->Move(dx, 0);
            }
            catch (const CollisionException & e)
            {
                std::cerr << e.what() << std::endl;
                this->packet_sender->SendPosition(this->player.GetLocation() + Vector<double>(0, 1, 0), false);
                Sleep(1000 / 20 * ticks_per_move);
                this->packet_sender->SendPosition(this->player.GetLocation() + Vector<double>(abs(dx) / dx, 0, 0), true);
                Sleep(1000 / 20 * ticks_per_move);
                continue;
            }

            if (!this->OnGround())
            {
                Sleep(1000 / 20 * ticks_per_move);
                this->MoveToGround(0.10);
            }

            Sleep(1000 / 20 * ticks_per_move);
        }


        for (double z = init.GetZ(); z != dest.GetZ(); z += dz)
        {
            Vector<double> current_location = this->player.GetLocation();

            try
            {
                this->Move(0, dz);
            }
            catch (const CollisionException & e)
            {
                std::cerr << e.what() << std::endl;
                this->packet_sender->SendPosition(this->player.GetLocation() + Vector<double>(0, 1, 0), false);
                Sleep(1000 / 20 * ticks_per_move);
                this->packet_sender->SendPosition(this->player.GetLocation() + Vector<double>(0, 0, abs(dz) / dz), true);
                Sleep(1000 / 20 * ticks_per_move);
                continue;
            }

            if (!this->OnGround())
            {
                Sleep(1000 / 20 * ticks_per_move);
                this->MoveToGround(0.10);
            }

            Sleep(1000 / 20 * ticks_per_move);
        }
    }

    void MCBot::AttackEntity(Entity entity)
    {
        this->packet_sender->SendUseEntity(entity.GetID(), UseEntityType::ATTACK);
    }

    void MCBot::RegisterEntity(Entity entity)
    {
        this->entities.insert(std::pair<int, Entity>(entity.GetID(), entity));
    }

    void MCBot::RemoveEntity(int id)
    {
        this->entities.erase(id);
    }

    bool MCBot::IsEntityRegistered(int id)
    {
        return this->entities.find(id) != this->entities.end();
    }

    void MCBot::RegisterPlayer(UUID uuid, EntityPlayer player)
    {
        this->uuid_to_player.insert(std::pair<UUID, EntityPlayer>(uuid, player));
    }

    void MCBot::RemovePlayer(EntityPlayer player)
    {
        this->RemovePlayer(player.GetUUID());
    }

    void MCBot::RemovePlayer(UUID uuid)
    {
        this->uuid_to_player.erase(uuid);
    }

    EntityPlayer& MCBot::GetPlayer(UUID uuid)
    {
        return this->uuid_to_player[uuid];
    }

    Entity& MCBot::GetEntity(int id)
    {
        return this->entities[id];
    }

    bool MCBot::IsPlayerRegistered(UUID uuid)
    {
        return this->uuid_to_player.find(uuid) != this->uuid_to_player.end();
    }

    void MCBot::LoadChunk(Chunk chunk)
    {
        this->chunks.insert(std::pair<std::pair<int, int>, Chunk>(std::pair<int, int>(chunk.GetX(), chunk.GetZ()), chunk));
    }

    Chunk& MCBot::GetChunk(int x, int z)
    {
        return this->chunks[std::pair<int, int>(x, z)];
    }

    Chunk& MCBot::GetChunk(Vector<int> location)
    {
        return this->chunks[std::pair<int, int>(location.GetX() >> 4, location.GetZ() >> 4)];
    }

    Chunk& MCBot::GetChunk(Vector<double> location)
    {
        int x = ((int)floor(location.GetX())) >> 4;
        int z = ((int)floor(location.GetZ())) >> 4;
        return this->chunks[std::pair<int, int>(x, z)];
    }

    std::list<Chunk> MCBot::GetChunks(int x, int z, unsigned int radius)
    {
        std::list<Chunk> chunks;

        for (int i = -((int)radius); i <= radius; i++)
            for (int j = -((int)radius); j <= radius; j++)
            {
                chunks.push_back(this->chunks[std::pair<int, int>(x + i, z + j)]);
            }

        return chunks;
    }

    Vector<double> MCBot::GetGroundLocation(Vector<double> location)
    {
        Chunk chunk = GetChunk(location);
        location.SetY(floor(location.GetY()));

        while (location.GetY() >= 0)
        {
            Block block = Block(chunk.GetBlockID(location));
            if (!block.IsWeak())
            {
                break;
            }
            location = location - Vector<double>(0, 1, 0);
        }

        return location;
    }

    void MCBot::MoveToGround(double speed)
    {
        if (this->OnGround())
        {
            return;
        }
        Vector<double> dest = this->GetGroundLocation(this->player.GetLocation()) + Vector<double>(0, 1, 0);
        this->MoveTo(dest, speed, true);
    }

    bool MCBot::OnGround()
    {
        Vector<double> player_location = this->player.GetLocation();
        Vector<double> ground_location = this->GetGroundLocation(player_location) + Vector<double>(0, 1, 0);

        return player_location.GetY() == ground_location.GetY();
    }

    void MCBot::Move(Vector<double> diff)
    {
        Vector<double> init = this->player.GetLocation();
        Vector<double> dest = init + diff;

        Chunk& dest_chunk = this->GetChunk(dest);
        Chunk& init_chunk = this->GetChunk(init);

        int dest_block_id = dest_chunk.GetBlockID(dest);

        // Collision detection
        if (dest_block_id != 0 && dest_block_id != 31)
        {
            throw CollisionException(dest_block_id, dest);
        }

        this->packet_sender->SendPosition(dest, true);
    }

    void MCBot::Move(double dx, double dz)
    {
        Vector<double> dr = Vector<double>(dx, 0, dz);
        Vector<double> init = this->player.GetLocation();
        Vector<double> dest = init + dr;

        Chunk& dest_chunk = this->GetChunk(dest);
        Chunk& init_chunk = this->GetChunk(init);
        Block dest_block = Block(dest_chunk.GetBlockID(dest));


        // Collision detection
        if (!dest_block.IsWeak())
        {
            std::cout << dest_block.GetID() << std::endl;
            throw CollisionException(dest_block, dest);
        }

        float yaw = dx > 0 ? -90 : dx < 0 ? 90 : dz > 0 ? 0 : dz < 0 ? 180 : 0;
        this->packet_sender->SendPositionLook(dest, yaw, 0, true);
    }

    void MCBot::LogDebug(std::string message)
    {
        if (!this->debug)
        {
            return;
        }

        std::cout << "\33[2K\r" << "[DEBUG] " << message << std::endl;
        std::cout << "> ";
    }

    void MCBot::LogError(std::string message)
    {
        std::cout << "\33[2K\r" << "[ERROR] " << message << std::endl;
        std::cout << "> ";
    }

    void MCBot::LogInfo(std::string message)
    {
        std::cout << "\33[2K\r" << "[INFO] " << message << std::endl;
        std::cout << "> ";
    }

    void MCBot::LogChat(std::string message)
    {
        std::cout << "\33[2K\r" << "[CHAT] " << message << std::endl;
        std::cout << "> ";
    }

    void MCBot::SetDebug(bool debug)
    {
        this->debug = debug;
    }

    void MCBot::SetState(State state)
    {
        this->state = state;
    }

    void MCBot::SetConnected(bool connected)
    {
        this->connected = connected;
    }

    void MCBot::SetSession(MojangSession session)
    {
        this->session = session;
    }

    MojangSession MCBot::GetSession()
    {
        return this->session;
    }

    std::list<Entity> MCBot::GetEntities()
    {
        typedef std::map<int, Entity> map_type;
        std::list<Entity> value_list;
        for (map_type::const_iterator it = this->entities.begin(); it != this->entities.end(); ++it)
        {
            value_list.push_back(it->second);
        }
        return value_list;
    }

    std::list<EntityPlayer> MCBot::GetPlayers()
    {
        typedef std::map<UUID, EntityPlayer> map_type;
        std::list<EntityPlayer> value_list;
        for (map_type::const_iterator it = this->uuid_to_player.begin(); it != this->uuid_to_player.end(); ++it)
        {
            value_list.push_back(it->second);
        }
        return value_list;
    }

    bool MCBot::IsConnected()
    {
        return this->connected;
    }

    bool MCBot::is_ready()
    {
        return this->ready;
    }

    State MCBot::GetState()
    {
        return this->state;
    }

    Socket& MCBot::GetSocket()
    {
        return this->sock;
    }

    PacketSender& MCBot::GetPacketSender()
    {
        return *this->packet_sender;
    }

    PacketReceiver& MCBot::GetPacketReceiver()
    {
        return *this->packet_receiver;
    }

    EntityPlayer& MCBot::GetPlayer()
    {
        return this->player;
    }

}
