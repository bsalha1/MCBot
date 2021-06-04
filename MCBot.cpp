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

    MCBot::MCBot() 
    {
        this->connected = false;
        this->packet_receiver = new PacketReceiver(this);
        this->packet_sender = new PacketSender(this);
        this->state = State::HANDSHAKE;
    }

    // std::mutex not copyable
    MCBot::MCBot(const MCBot& origin) : state_mutex()
    {
        this->connected = origin.connected;
        this->packet_receiver = origin.packet_receiver;
        this->packet_sender = origin.packet_sender;
        this->state = origin.state;
    }

    MCBot::~MCBot()
    {
        delete this->packet_receiver;
        delete this->packet_sender;
        this->sock.CleanupEncryption();
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
            this->logger.LogError("Failed to resolve " + hostname_s);
            print_winsock_error();
            WSACleanup();
            return -1;
        }

        char* address_string = inet_ntoa(((struct sockaddr_in*) result->ai_addr)->sin_addr);
        std::string address_string_s = address_string;

        this->logger.LogInfo("Resolved " + hostname_s + " to " + address_string_s);

        // Connect //
        this->sock = Socket(socket(result->ai_family, result->ai_socktype, result->ai_protocol));
        this->logger.LogInfo("Connecting to " + address_string_s + ":" + port);
        if (this->sock.ConnectSocket(result) < 0)
        {
            print_winsock_error();
            WSACleanup();
            return -1;
        }

        this->connected = true;
        this->logger.LogInfo("Connected to " + address_string_s + ":" + port);

        return 0;
    }

    void MCBot::LoginToServer(char* hostname, char* port)
    {
        this->packet_sender->SendHandshake(hostname, atoi(port));
        this->packet_sender->SendLoginStart();
    }

    int MCBot::LoginToMojang(char* email, char* password)
    {
        if (this->packet_sender->LoginMojang(email, password) < 0)
        {
            return -1;
        }

        if (this->packet_sender->VerifyAccessToken() < 0)
        {
            return -1;
        }

        return 0;
    }

    void MCBot::AwaitState(State state)
    {
        if (this->state == state)
        {
            return;
        }

        std::unique_lock<std::mutex> lk(this->state_mutex);

        cv.wait(lk, [this, state] {
            return this->state == state;
        });
    }
    
    std::thread MCBot::StartPacketReceiverThread()
    {
        std::thread recv_thread([this]() {
            while (this->connected)
            {
                Sleep(1);
                this->packet_receiver->ReadAndHandleNextPacket();
            }
        });

        return recv_thread;
    }

    std::thread MCBot::StartPositionThread()
    {
        std::thread position_thread([this]() {
            while (this->connected)
            {
                Sleep(1000 / TPS);
                if (this->player.GetPing() == -1)
                {
                    continue;
                }

                auto location = this->player.GetLocation();

                // Chunk has to have been loaded
                if (!this->chunk_registry.IsValueRegistered(std::pair<int, int>((int)location.GetX() >> 4, (int)location.GetZ() >> 4)))
                {
                    continue;
                }

                // Determine if player is on the ground
                bool on_ground = true;
                if (BlockUtils::CanPassThrough(this->GetChunk(location).GetBlockID(location - Vector<double>(0, 1, 0))))
                {
                    on_ground = false;
                }

                this->packet_sender->SendPositionLook(location, this->player.GetYaw(), this->player.GetPitch(), on_ground);
            }
        });

        return position_thread;
    }

    void MCBot::MoveTo(Vector<double> destination, double speed)
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
                this->player.UpdateLocation(destination);
                return;
            }

            try
            {
                this->Move(direction);
            }
            catch (const CollisionException & e)
            {
                std::cerr << e.what() << std::endl;
                this->player.UpdateLocation(direction + Vector<double>(0, 1, 0));
            }

            Sleep(1000 / 20);
        }
    }

    void MCBot::MoveTo(double x, double z, int ms_per_block)
    {
        // Clean up to make math easier
        x = floor(x) + 0.5;
        z = floor(z) + 0.5;

        Vector<double> dest(x, 0, z);
        Vector<double> init = this->player.GetLocation();

        auto diff = dest - init;
        auto diff_x = diff.GetX();
        auto diff_z = diff.GetZ();
        double dx = diff_x == 0 ? 0 : abs(diff_x) / diff_x;
        double dz = diff_z == 0 ? 0 : abs(diff_z) / diff_z;

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
                this->player.UpdateLocation(this->player.GetLocation() + Vector<double>(0, 1, 0));
                Sleep(ms_per_block);
                this->player.UpdateLocation(this->player.GetLocation() + Vector<double>(abs(dx) / dx, 0, 0));
                Sleep(ms_per_block);
                continue;
            }

            if (!this->OnGround())
            {
                Sleep(ms_per_block);
                this->MoveToGround(0.10);
            }

            Sleep(ms_per_block);
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
                this->player.UpdateLocation(this->player.GetLocation() + Vector<double>(0, 1, 0));
                Sleep(ms_per_block);
                this->player.UpdateLocation(this->player.GetLocation() + Vector<double>(0, 0, abs(dz) / dz));
                Sleep(ms_per_block);
                continue;
            }

            if (!this->OnGround())
            {
                Sleep(ms_per_block);
                this->MoveToGround(0.10);
            }

            Sleep(ms_per_block);
        }
    }

    void MCBot::AttackEntity(Entity entity)
    {
        this->packet_sender->SendUseEntity(entity.GetID(), UseEntityType::ATTACK);
    }

    Registry<int, Entity>& MCBot::GetEntityRegistry()
    {
        return this->entity_registry;
    }

    Registry<UUID, EntityPlayer>& MCBot::GetPlayerRegistry()
    {
        return this->player_registry;
    }

    Registry<std::pair<int, int>, Chunk>& MCBot::GetChunkRegistry()
    {
        return this->chunk_registry;
    }

    Chunk& MCBot::GetChunk(int x, int z)
    {
        return this->chunk_registry.GetValue(std::pair<int, int>(x, z));
    }

    Chunk& MCBot::GetChunk(Vector<int> location)
    {
        return this->chunk_registry.GetValue(std::pair<int, int>(location.GetX() >> 4, location.GetZ() >> 4));
    }

    Chunk& MCBot::GetChunk(Vector<double> location)
    {
        int x = ((int)floor(location.GetX())) >> 4;
        int z = ((int)floor(location.GetZ())) >> 4;
        return this->chunk_registry.GetValue(std::pair<int, int>(x, z));
    }

    std::list<Chunk> MCBot::GetChunks(int x, int z, int radius)
    {
        std::list<Chunk> chunks;

        radius = abs(radius);
        for (int i = -radius; i <= radius; i++)
            for (int j = -radius; j <= radius; j++)
            {
                chunks.push_back(this->chunk_registry.GetValue(std::pair<int, int>(x + i, z + j)));
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
            if (!block.CanPassThrough())
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
        auto dest = this->GetGroundLocation(this->player.GetLocation()) + Vector<double>(0, 1, 0);
        this->MoveTo(dest, speed);
    }

    bool MCBot::OnGround()
    {
        auto player_location = this->player.GetLocation();
        auto ground_location = this->GetGroundLocation(player_location) + Vector<double>(0, 1, 0);

        return player_location.GetY() == ground_location.GetY();
    }

    void MCBot::UpdatePlayerInfo(PlayerInfoAction action, int length, Packet& packet)
    {
        for (int i = 0; i < length; i++)
        {
            UUID uuid = PacketDecoder::ReadUUID(packet);

            this->logger.LogDebug("\tPlayer Update (" + uuid.ToString() + "): " + StringUtils::to_string(action));

            switch (action)
            {
            case PlayerInfoAction::ADD_PLAYER:
            {
                std::string name = PacketDecoder::ReadString(packet);
                int properties_length = PacketDecoder::ReadVarInt(packet);
                std::list<PlayerProperty> properties = PacketDecoder::ReadPropertyArray(properties_length, packet);
                Gamemode gamemode = (Gamemode)PacketDecoder::ReadVarInt(packet);
                int ping = PacketDecoder::ReadVarInt(packet);
                bool has_display_name = PacketDecoder::ReadBoolean(packet);
                std::string display_name = has_display_name ? PacketDecoder::ReadString(packet) : name;

                EntityPlayer player = EntityPlayer(-1, uuid, name, properties, gamemode, ping, display_name);

                // If this player is the bot player
                if (name == this->session.GetUsername())
                {
                    // TODO: just copy the player variable over
                    this->player.SetName(name);
                    this->player.SetUUID(uuid);
                    this->player.SetProperties(properties);
                    this->player.SetGamemode(gamemode);
                    this->player.SetPing(ping);
                    this->player.SetDisplayName(display_name);
                }

                this->player_registry.RegisterValue(uuid, player);

                this->logger.LogDebug("\t\tName: " + name + '\n'
                    + "\t\tGamemode: " + StringUtils::to_string(gamemode) + '\n'
                    + "\t\tPing: " + std::to_string(ping));
                break;
            }

            case PlayerInfoAction::UPDATE_GAMEMODE:
            {
                try
                {
                    EntityPlayer& player = this->player_registry.GetValue(uuid);
                }
                catch (...)
                {
                    this->logger.LogError("Failed to find player of UUID " + uuid.ToString());
                    return;
                }
                Gamemode gamemode = (Gamemode)PacketDecoder::ReadVarInt(packet);
                player.SetGamemode(gamemode);
                break;
            }

            case PlayerInfoAction::UPDATE_LATENCY:
            {
                try
                {
                    EntityPlayer& player = this->player_registry.GetValue(uuid);
                }
                catch (...)
                {
                    this->logger.LogError("Failed to find player of UUID " + uuid.ToString());
                    return;
                }
                int ping = PacketDecoder::ReadVarInt(packet);

                player.SetPing(ping);
                break;
            }

            case PlayerInfoAction::UPDATE_DISPLAY_NAME:
            {
                try
                {
                    EntityPlayer& player = this->player_registry.GetValue(uuid);
                }
                catch (...)
                {
                    this->logger.LogError("Failed to find player of UUID " + uuid.ToString());
                    return;
                }

                bool has_display_name = PacketDecoder::ReadBoolean(packet);
                std::string display_name = has_display_name ? PacketDecoder::ReadString(packet) : "";
                if (has_display_name)
                {
                    player.SetDisplayName(display_name);
                }

                break;
            }

            case PlayerInfoAction::REMOVE_PLAYER:
            {
                this->player_registry.RemoveValue(uuid);
                break;
            }
            default:
                break;
            }
        }
    }

    void MCBot::UpdatePlayerRotation(float yaw, float pitch)
    {
        this->player.UpdateRotation(yaw, pitch);
    }

    void MCBot::UpdatePlayerInventory(std::array<Slot, 45> player_inventory)
    {
        this->player.UpdateInventory(player_inventory);
    }

    void MCBot::UpdatePlayerLocation(Vector<double> location)
    {
        this->player.UpdateLocation(location);
    }

    int MCBot::SendPacket(Packet packet)
    {
        return this->sock.SendPacket(packet.data, packet.offset);
    }

    void MCBot::InitEncryption(uint8_t* key, uint8_t* iv)
    {
        this->sock.InitEncryption(key, iv);
    }

    void MCBot::InitCompression(int max_uncompressed_length)
    {
        this->sock.InitCompression(max_uncompressed_length);
    }

    void MCBot::Move(Vector<double> diff)
    {
        auto init = this->player.GetLocation();
        auto dest = init + diff;

        Chunk dest_chunk = this->GetChunk(dest);
        Chunk init_chunk = this->GetChunk(init);

        int dest_block_id = dest_chunk.GetBlockID(dest);

        // Collision detection
        if (dest_block_id != 0 && dest_block_id != 31)
        {
            throw CollisionException(dest_block_id, dest);
        }

        this->player.UpdateLocation(dest);
    }

    void MCBot::Move(double dx, double dz)
    {
        Vector<double> dr(dx, 0, dz);
        auto init = this->player.GetLocation();
        auto dest = init + dr;

        Chunk dest_chunk = this->GetChunk(dest);
        Chunk init_chunk = this->GetChunk(init);
        Block dest_block(dest_chunk.GetBlockID(dest));

        // Collision detection
        if (!dest_block.CanPassThrough())
        {
            throw CollisionException(dest_block, dest);
        }

        float yaw = dx > 0.0 ? -90.0F : dx < 0.0 ? 90.0F : dz < 0.0 ? 180.0F : 0.0F;
        this->player.UpdateLocation(dest);
        this->player.UpdateRotation(yaw, 0);
    }

    void MCBot::SetState(State state)
    {
        this->state = state;
        this->cv.notify_all();
    }

    void MCBot::SetConnected(bool connected)
    {
        this->connected = connected;
    }

    void MCBot::SetSession(MojangSession session)
    {
        this->session = session;
    }

    MojangSession MCBot::GetSession() const
    {
        return this->session;
    }

    bool MCBot::IsConnected() const
    {
        return this->connected;
    }

    State MCBot::GetState() const
    {
        return this->state;
    }

    Socket MCBot::GetSocket()
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

    Logger& MCBot::GetLogger()
    {
        return this->logger;
    }

    EntityPlayer MCBot::GetPlayer() const
    {
        return this->player;
    }

}
