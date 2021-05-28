#include "MCBot.h"
#include "PacketReceiver.h"
#include "PacketSender.h"
#include "StringUtils.h"

namespace mcbot
{
    static std::string GetRandomHexBytes(std::size_t num_bytes)
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

    PacketReceiver::PacketReceiver(MCBot* bot) : player(&(bot->GetPlayer()))
    {
        this->bot = bot;
    }

    PacketReceiver::PacketReceiver() : player(NULL)
    {
        this->bot = NULL;
    }


    int PacketReceiver::ReadNextVarInt()
    {
        int num_read = 0;
        int result = 0;
        char read;
        int i = 0;
        do {

            uint8_t packet[2] = { 0 };
            size_t bytes_read = this->bot->GetSocket().recv_packet(packet, 1);
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

    void PacketReceiver::RecvPacket()
    {
        int length = this->ReadNextVarInt();
        this->bot->LogDebug("Length: " + std::to_string(length));

        uint8_t* packet;
        int decompressed_length = 0;
        if (this->compression_enabled)
        {
            decompressed_length = this->ReadNextVarInt();
            length -= PacketEncoder::GetVarIntNumBytes(decompressed_length);
            this->bot->LogDebug("Decompressed length: " + std::to_string(decompressed_length));
        }

        int packet_size;
        if (decompressed_length == 0)
        {
            packet = new uint8_t[length]{ 0 };
            packet_size = this->ReadNextPacket(length, packet);
        }
        else
        {
            packet = new uint8_t[decompressed_length]{ 0 };
            packet_size = this->ReadNextPacket(length, packet, decompressed_length);
        }

        size_t offset = 0;
        int packet_id = PacketDecoder::ReadVarInt(packet, offset);
        this->bot->LogDebug("ID: " + std::to_string(packet_id));

        HandleRecvPacket(packet_id, packet, packet_size, offset);

        delete[] packet;
    }

    int PacketReceiver::ReadNextPacket(int length, uint8_t* packet, int decompressed_length)
    {
        int bytes_read = this->bot->GetSocket().recv_packet(packet, length, decompressed_length);
        if (bytes_read < 0)
        {
            this->bot->LogError("Failed to receive packet");
            packet = NULL;
            return bytes_read;
        }
        this->bot->LogDebug("Received Packet: " + std::to_string(bytes_read) + "bytes");

        return bytes_read;
    }

    void PacketReceiver::HandleRecvPacket(int packet_id, uint8_t* packet, int length, size_t& offset)
    {
        if (this->bot->GetState() == State::HANDSHAKE)
        {

        }
        else if (this->bot->GetState() == State::STATUS)
        {

        }
        else if (this->bot->GetState() == State::LOGIN)
        {
            switch (packet_id)
            {
            case 0x00:
                this->RecvLoginDisconnect(packet, length, offset);
                break;
            case 0x01:
                this->RecvEncryptionRequest(packet, length, offset);
                break;
            case 0x02:
                this->RecvLoginSuccess(packet, length, offset);
                break;
            case 0x03:
                this->RecvSetCompression(packet, length, offset);
                break;
            default:
                this->bot->LogError("Unhandled " + StringUtils::to_string(this->bot->GetState()) + " packet id: " + std::to_string(packet_id));
            }
        }
        else if (this->bot->GetState() == State::PLAY)
        {
            switch (packet_id)
            {
            case 0x00:
                this->RecvKeepAlive(packet, length, offset);
                break;
            case 0x01:
                this->RecvJoinServer(packet, length, offset);
                break;
            case 0x02:
                this->RecvChatMessage(packet, length, offset);
                break;
            case 0x03:
                this->RecvUpdateTime(packet, length, offset);
                break;
            case 0x04:
                this->RecvEntityEquipment(packet, length, offset);
                break;
            case 0x05:
                this->RecvSpawnPosition(packet, length, offset);
                break;
            case 0x06:
                this->RecvUpdateHealth(packet, length, offset);
                break;
            case 0x08:
                this->RecvPosition(packet, length, offset);
                break;
            case 0x09:
                this->RecvHeldItemSlot(packet, length, offset);
                break;
            case 0x0A:
                this->RecvBed(packet, length, offset);
                break;
            case 0x0B:
                this->RecvAnimation(packet, length, offset);
                break;
            case 0x0C:
                this->RecvNamedEntitySpawn(packet, length, offset);
                break;
            case 0x0D:
                this->RecvCollect(packet, length, offset);
                break;
            case 0x0E:
                this->RecvSpawnEntity(packet, length, offset);
                break;
            case 0x0F:
                this->RecvSpawnEntityLiving(packet, length, offset);
                break;
            case 0x10:
                this->RecvSpawnEntityPainting(packet, length, offset);
                break;
            case 0x11:
                this->RecvSpawnEntityExperienceOrb(packet, length, offset);
                break;
            case 0x12:
                this->RecvEntityVelocity(packet, length, offset);
                break;
            case 0x13:
                this->RecvEntityDestroy(packet, length, offset);
                break;
            case 0x14:
                this->RecvEntity(packet, length, offset);
                break;
            case 0x15:
                this->RecvRelEntityMove(packet, length, offset);
                break;
            case 0x16:
                this->RecvEntityLook(packet, length, offset);
                break;
            case 0x17:
                this->RecvRelEntityMoveLook(packet, length, offset);
                break;
            case 0x18:
                this->RecvEntityTeleport(packet, length, offset);
                break;
            case 0x19:
                this->RecvEntityHeadLook(packet, length, offset);
                break;
            case 0x1A:
                this->RecvEntityStatus(packet, length, offset);
                break;
            case 0x1C:
                this->RecvEntityMetadata(packet, length, offset);
                break;
            case 0x1D:
                this->RecvEntityEffect(packet, length, offset);
                break;
            case 0x1F:
                this->RecvExperience(packet, length, offset);
                break;
            case 0x20:
                this->RecvEntityAttributes(packet, length, offset);
                break;
            case 0x21:
                this->RecvMapChunk(packet, length, offset);
                break;
            case 0x22:
                this->RecvMultiBlockChange(packet, length, offset);
                break;
            case 0x23:
                this->RecvBlockChange(packet, length, offset);
                break;
            case 0x25:
                this->RecvBlockBreakAnimation(packet, length, offset);
                break;
            case 0x28:
                this->RecvWorldEvent(packet, length, offset);
                break;
            case 0x26:
                this->RecvMapChunkBulk(packet, length, offset);
                break;
            case 0x29:
                this->RecvNamedSoundEffect(packet, length, offset);
                break;
            case 0x2A:
                this->RecvWorldParticles(packet, length, offset);
                break;
            case 0x2B:
                this->RecvGameStateChange(packet, length, offset);
                break;
            case 0x2C:
                this->RecvSpawnEntityWeather(packet, length, offset);
                break;
            case 0x2F:
                this->RecvSetSlot(packet, length, offset);
                break;
            case 0x30:
                this->RecvWindowItems(packet, length, offset);
                break;
            case 0x32:
                this->RecvTransaction(packet, length, offset);
                break;
            case 0x33:
                this->RecvUpdateSign(packet, length, offset);
                break;
            case 0x35:
                this->RecvTileEntityData(packet, length, offset);
                break;
            case 0x37:
                this->RecvStatistics(packet, length, offset);
                break;
            case 0x38:
                this->RecvPlayerInfo(packet, length, offset);
                break;
            case 0x39:
                this->RecvAbilities(packet, length, offset);
                break;
            case 0x3B:
                this->RecvScoreboardObjective(packet, length, offset);
                break;
            case 0x3C:
                this->RecvUpdateScoreboardScore(packet, length, offset);
                break;
            case 0x3D:
                this->RecvDisplayScoreboard(packet, length, offset);
                break;
            case 0x3E:
                this->RecvScoreboardTeam(packet, length, offset);
                break;
            case 0x3F:
                this->RecvPluginMessage(packet, length, offset);
                break;
            case 0x40:
                this->RecvPlayDisconnect(packet, length, offset);
                break;
            case 0x41:
                this->RecvServerDifficulty(packet, length, offset);
                break;
            case 0x44:
                this->RecvWorldBorder(packet, length, offset);
                break;
            case 0x45:
                this->RecvTitle(packet, length, offset);
                break;
            case 0x47:
                this->RecvPlayerListHeaderFooter(packet, length, offset);
                break;
            default:
                this->bot->LogError("Unhandled " + StringUtils::to_string(this->bot->GetState()) + " packet id: " + std::to_string(packet_id));
            }
        }
        else
        {
            this->bot->LogError("Unknown state: " + StringUtils::to_string(this->bot->GetState()));
        }
    }

    void PacketReceiver::RecvLoginDisconnect(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketLoginOutDisconnect...");

        std::cout << packet << std::endl;

        this->bot->SetConnected(false);
        this->bot->LogInfo("Disconnected");
    }

    void PacketReceiver::RecvSetCompression(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketLoginOutSetCompression...");
        int max_uncompressed_length = PacketDecoder::ReadVarInt(packet, offset);

        this->bot->LogDebug("\tMax Uncompressed Length: " + std::to_string(max_uncompressed_length));
        this->compression_enabled = true;
        this->bot->GetSocket().initialize_compression(max_uncompressed_length);
        this->bot->LogDebug("<<< COMPRESSION ENABLED <<<");
    }

    void PacketReceiver::RecvEncryptionRequest(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketLoginOutEncryptionRequest...");

        // Server ID //
        std::string server_id = PacketDecoder::ReadString(packet, offset);

        // Public Key //
        int public_key_length = PacketDecoder::ReadVarInt(packet, offset);
        uint8_t* public_key = new uint8_t[public_key_length];
        PacketDecoder::ReadByteArray(public_key, public_key_length, packet, offset);

        // Verify Token //
        int verify_token_length = PacketDecoder::ReadVarInt(packet, offset);
        uint8_t* verify_token = new uint8_t[verify_token_length];
        PacketDecoder::ReadByteArray(verify_token, verify_token_length, packet, offset);

        std::string shared_secret = GetRandomHexBytes(16);

        // Save Session //
        // - So Yggdrasil authentication doesn't kick us!
        this->bot->LogDebug("Saving session...");
        if (this->bot->GetPacketSender().SendSession(server_id, shared_secret, public_key, public_key_length) < 0)
        {
            this->bot->LogError("Invalid session!");
        }
        else
        {
            this->bot->GetPacketSender().SendEncryptionResponse(public_key, public_key_length, verify_token, verify_token_length, shared_secret);
        }
    }

    void PacketReceiver::RecvLoginSuccess(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayLoginOutSuccess...");

        std::string uuid_string = PacketDecoder::ReadString(packet, offset);
        std::string username = PacketDecoder::ReadString(packet, offset);

        this->bot->LogDebug(
            "UUID: " + uuid_string + '\n' +
            "\tUsername: " + username);

        this->bot->LogInfo("Successfully logged in!");

        this->bot->SetState(State::PLAY);
    }

    void PacketReceiver::RecvPlayDisconnect(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutKickDisconnect...");

        std::string reason = PacketDecoder::ReadString(packet, offset);

        this->bot->LogInfo("Disconnected: " + reason);

        this->bot->SetConnected(false);
    }

    void PacketReceiver::RecvKeepAlive(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutKeepAlive...");

        int id = PacketDecoder::ReadVarInt(packet, offset);

        this->bot->GetPacketSender().SendKeepAlive(id);
    }

    void PacketReceiver::RecvJoinServer(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutJoinServer...");

        int entity_id = PacketDecoder::ReadInt(packet, offset);
        Gamemode gamemode = (Gamemode) PacketDecoder::ReadByte(packet, offset);
        Dimension dimension = (Dimension) PacketDecoder::ReadByte(packet, offset);
        Difficulty difficulty = (Difficulty) PacketDecoder::ReadByte(packet, offset);
        uint8_t max_players = PacketDecoder::ReadByte(packet, offset);
        std::string level_type = PacketDecoder::ReadString(packet, offset);
        bool reduced_debug_info = PacketDecoder::ReadBoolean(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tGamemode: " + StringUtils::to_string(gamemode) +
            "\n\tDimension: " + StringUtils::to_string(dimension) +
            "\n\tDifficulty: " + StringUtils::to_string(difficulty) +
            "\n\tMax Players: " + std::to_string((int)max_players) +
            "\n\tLevel Type: " + level_type +
            "\n\tReduced Debug Info: " + std::to_string(reduced_debug_info));


    }

    void PacketReceiver::RecvChatMessage(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutChat...");

        std::string chat_data = PacketDecoder::ReadString(packet, offset);
        ChatPosition position = (ChatPosition) PacketDecoder::ReadByte(packet, offset);

        this->bot->LogDebug(
            "Chat Data: " + chat_data +
            "\n\tPosition: " + StringUtils::to_string(position));


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

        this->bot->LogChat(chat_message);
    }

    void PacketReceiver::RecvUpdateTime(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutUpdateTime...");

        int64_t world_age = (int64_t)PacketDecoder::ReadLong(packet, offset);
        int64_t time_of_day = (int64_t)PacketDecoder::ReadLong(packet, offset);

        this->bot->LogDebug(
            "World Age: " + std::to_string(world_age) +
            "\n\tTime of Day: " + std::to_string(time_of_day));
    }

    void PacketReceiver::RecvEntityEquipment(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutEntityEquipment...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        short slot = PacketDecoder::ReadShort(packet, offset);
        Slot item = PacketDecoder::ReadSlot(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tSlot: " + std::to_string(slot) +
            "\n\tItem: " + item.ToString());
    }

    void PacketReceiver::RecvSpawnPosition(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutSpawnPosition...");

        Position location = PacketDecoder::ReadPosition(packet, offset);

        this->bot->LogDebug("Location: " + location.ToString());
    }

    void PacketReceiver::RecvUpdateHealth(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutUpdateHealth...");

        float health = PacketDecoder::ReadFloat(packet, offset);
        int food = PacketDecoder::ReadVarInt(packet, offset);
        float food_saturation = PacketDecoder::ReadFloat(packet, offset);

        this->bot->LogDebug("Health: " + std::to_string(health) +
            "\n\tFood: " + std::to_string(food) +
            "\n\tSaturation: " + std::to_string(food_saturation));
    }


    void PacketReceiver::RecvPosition(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutPosition...");

        Vector<double> position = PacketDecoder::ReadVector<double>(packet, offset);
        float yaw = PacketDecoder::ReadFloat(packet, offset);
        float pitch = PacketDecoder::ReadFloat(packet, offset);
        uint8_t flags = PacketDecoder::ReadByte(packet, offset);

        this->bot->LogDebug("Position: " + position.ToString() +
            "\n\tYaw: " + std::to_string(yaw) +
            "\n\tPitch: " + std::to_string(pitch) +
            "\n\tFlags: " + std::to_string((int)flags));

        this->bot->GetPlayer().UpdateLocation(position);
        this->bot->GetPlayer().UpdateRotation(yaw, pitch);
    }

    void PacketReceiver::RecvHeldItemSlot(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutHeldItemSlot...");

        uint8_t held_item_slot = PacketDecoder::ReadByte(packet, offset);

        this->bot->LogDebug("Held Item Slot: " + std::to_string((int)held_item_slot));
    }

    void PacketReceiver::RecvBed(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutBed...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        Position position = PacketDecoder::ReadPosition(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tLocation: " + position.ToString());
    }

    void PacketReceiver::RecvAnimation(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutAnimation...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        Position position = PacketDecoder::ReadPosition(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tLocation: " + position.ToString());
    }

    void PacketReceiver::RecvNamedEntitySpawn(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutNamedEntitySpawn...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        UUID uuid = PacketDecoder::ReadUUID(packet, offset);
        int x = PacketDecoder::ReadInt(packet, offset);
        int y = PacketDecoder::ReadInt(packet, offset);
        int z = PacketDecoder::ReadInt(packet, offset);
        Vector<double> position = Vector<double>(x / 32.0, y / 32.0, z / 32.0);

        uint8_t pitch = PacketDecoder::ReadByte(packet, offset);
        uint8_t yaw = PacketDecoder::ReadByte(packet, offset);
        short current_item = PacketDecoder::ReadShort(packet, offset);
        EntityMetaData meta_data = PacketDecoder::ReadMetaData(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tUUID: " + uuid.ToString() +
            "\n\tLocation: " + position.ToString());

        if (!this->bot->IsPlayerRegistered(uuid))
        {
            EntityPlayer player = EntityPlayer(entity_id, uuid);
            player.UpdateLocation(position);
            player.UpdateRotation(yaw, pitch);
            this->bot->RegisterPlayer(uuid, player);
        }
        else
        {
            EntityPlayer& player = this->bot->GetPlayer(uuid);
            player.SetID(entity_id);
            player.UpdateLocation(position);
            player.UpdateRotation(yaw, pitch);
        }
    }

    void PacketReceiver::RecvCollect(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutCollect...");

        int collected_id = PacketDecoder::ReadVarInt(packet, offset);
        int collector_id = PacketDecoder::ReadVarInt(packet, offset);

        this->bot->LogDebug(
            "Collected ID: " + std::to_string(collected_id) +
            "\n\tCollector ID: " + std::to_string(collector_id));
    }

    void PacketReceiver::RecvSpawnEntity(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutSpawnEntity...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        EntityType type = (EntityType)PacketDecoder::ReadByte(packet, offset);
        Vector<int> position = PacketDecoder::ReadVector<int>(packet, offset);
        Vector<double> position1 = Vector<double>(position.GetX() / 32.0, position.GetY() / 32.0, position.GetZ() / 32.0);

        uint8_t pitch = PacketDecoder::ReadByte(packet, offset);
        uint8_t yaw = PacketDecoder::ReadByte(packet, offset);
        int data = PacketDecoder::ReadInt(packet, offset);

        if (data > 0)
        {
            Vector<short> motion = PacketDecoder::ReadVector<short>(packet, offset);
        }

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tLocation:" + position.ToString());

        if (this->bot->IsEntityRegistered(entity_id))
        {
            this->bot->GetEntity(entity_id).UpdateLocation(position1);
        }
        else
        {
            Entity entity = Entity(type, entity_id);
            entity.UpdateLocation(position1);
            this->bot->RegisterEntity(entity);
        }
    }

    void PacketReceiver::RecvSpawnEntityLiving(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutSpawnEntityLiving...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        EntityType type = (EntityType)PacketDecoder::ReadByte(packet, offset);
        Vector<int> position = PacketDecoder::ReadVector<int>(packet, offset);
        Vector<double> position1 = Vector<double>(position.GetX() / 32.0, position.GetY() / 32.0, position.GetZ() / 32.0);

        uint8_t pitch = PacketDecoder::ReadByte(packet, offset);
        uint8_t yaw = PacketDecoder::ReadByte(packet, offset);
        uint8_t head_pitch = PacketDecoder::ReadByte(packet, offset);
        Vector<short> motion = PacketDecoder::ReadVector<short>(packet, offset);
        EntityMetaData meta_data = PacketDecoder::ReadMetaData(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tLocation:" + position.ToString());

        if (this->bot->IsEntityRegistered(entity_id))
        {
            this->bot->GetEntity(entity_id).UpdateLocation(position1);
        }
        else
        {
            EntityLiving entity = EntityLiving(type, entity_id);
            entity.UpdateLocation(position1);
            this->bot->RegisterEntity(entity);
        }
    }

    void PacketReceiver::RecvSpawnEntityPainting(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutSpawnEntityPainting...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        std::string title = PacketDecoder::ReadString(packet, offset);
        Position location = PacketDecoder::ReadPosition(packet, offset);
        uint8_t direction = PacketDecoder::ReadByte(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tLocation: " + location.ToString() +
            "\n\tTitle: " + title +
            "\n\tDirection: " + std::to_string((int)direction));

        this->bot->RegisterEntity(Entity(EntityType::PAINTING, entity_id));
    }

    void PacketReceiver::RecvSpawnEntityExperienceOrb(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutSpawnEntityExperienceOrb...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        Vector<int> motion = PacketDecoder::ReadVector<int>(packet, offset);
        short count = PacketDecoder::ReadShort(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tPosition: " + motion.ToString() +
            "\n\tCount: " + std::to_string(count));

        this->bot->RegisterEntity(Entity(EntityType::EXPERIENCE_ORB, entity_id));
    }

    void PacketReceiver::RecvEntityVelocity(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutEntityVelocity...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        Vector<short> motion = PacketDecoder::ReadVector<short>(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tVelocity: " + motion.ToString());
    }

    void PacketReceiver::RecvEntityDestroy(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutEntityDestroy...");

        int count = PacketDecoder::ReadVarInt(packet, offset);

        std::list<int> entity_ids = std::list <int>();
        for (int i = 0; i < count; i++)
        {
            entity_ids.push_back(PacketDecoder::ReadVarInt(packet, offset));
        }

        this->bot->LogDebug(
            "Entity Count: " + std::to_string(count));

        for (int id : entity_ids)
        {
            Entity& entity = this->bot->GetEntity(id);
            entity.Die();
            this->bot->RemoveEntity(id);

            if (entity.GetEntityType() == EntityType::PLAYER)
            {
                this->bot->RemovePlayer(static_cast<EntityPlayer&>(entity));
            }
        }
    }

    void PacketReceiver::RecvEntity(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutEntity...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id));

        this->bot->RegisterEntity(Entity(entity_id));
    }

    void PacketReceiver::RecvRelEntityMove(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutRelEntityMove...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        Vector<int8_t> dmot = PacketDecoder::ReadVector<int8_t>(packet, offset);
        bool on_ground = PacketDecoder::ReadBoolean(packet, offset);

        Vector<double> dr = Vector<double>(
            dmot.GetX() / 32.0, dmot.GetY() / 32.0, dmot.GetZ() / 32.0);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tChange in Motion: " + dr.ToString() +
            "\n\tOn Ground: " + std::to_string(on_ground));

        this->bot->GetEntity(entity_id).UpdateMotion(dr);
    }

    void PacketReceiver::RecvEntityLook(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutEntityLook...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        uint8_t yaw = PacketDecoder::ReadByte(packet, offset);
        uint8_t pitch = PacketDecoder::ReadByte(packet, offset);
        bool on_ground = PacketDecoder::ReadBoolean(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tYaw: " + std::to_string((int)yaw) +
            "\n\tPitch: " + std::to_string((int)pitch) +
            "\n\tOn Ground: " + std::to_string(on_ground));

        this->bot->GetEntity(entity_id).UpdateRotation(yaw, pitch);
    }

    void PacketReceiver::RecvRelEntityMoveLook(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutRelEntityMoveLook...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        Vector<int8_t> dmot = PacketDecoder::ReadVector<int8_t>(packet, offset);
        Vector<double> dr = Vector<double>(dmot.GetX() / 32.0, dmot.GetY() / 32.0, dmot.GetZ() / 32.0);
        double yaw = PacketDecoder::ReadByte(packet, offset) * 2 * 3.14159 / 256;
        uint8_t pitch = PacketDecoder::ReadByte(packet, offset) * 2 * 3.14159 / 256;
        bool on_ground = PacketDecoder::ReadBoolean(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tChange in Motion: " + dr.ToString() +
            "\n\tYaw: " + std::to_string(yaw) +
            "\n\tPitch: " + std::to_string(pitch) +
            "\n\tOn Ground: " + std::to_string(on_ground));

        this->bot->GetEntity(entity_id).UpdateMotion(dr);
        this->bot->GetEntity(entity_id).UpdateRotation(yaw, pitch);
    }

    void PacketReceiver::RecvEntityTeleport(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutEntityTeleport...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        Vector<int> position = PacketDecoder::ReadVector<int>(packet, offset);
        Vector<double> position1 = Vector<double>(position.GetX() / 32.0, position.GetY() / 32.0, position.GetZ() / 32.0);

        double yaw = PacketDecoder::ReadByte(packet, offset) * 2 * 3.14159 / 256;
        double pitch = PacketDecoder::ReadByte(packet, offset) * 2 * 3.14159 / 256;
        bool on_ground = PacketDecoder::ReadBoolean(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tChange in Motion: " + position.ToString() +
            "\n\tYaw: " + std::to_string(yaw) +
            "\n\tPitch: " + std::to_string(pitch) +
            "\n\tOn Ground: " + std::to_string(on_ground));

        this->bot->GetEntity(entity_id).UpdateLocation(position1);
        this->bot->GetEntity(entity_id).UpdateRotation(yaw, pitch);
    }

    void PacketReceiver::RecvEntityHeadLook(uint8_t* packet, size_t length, size_t& offset)
    {
        // TODO: find actual name of packet
        this->bot->LogDebug("<<< Handling PacketPlayOutEntityHeadLook...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        double yaw = PacketDecoder::ReadByte(packet, offset) * 2 * 3.14159 / 256;

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tAngle: " + std::to_string(yaw));

        this->bot->GetEntity(entity_id).UpdateYaw(yaw);
    }

    void PacketReceiver::RecvEntityStatus(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutEntityStatus...");

        int entity_id = PacketDecoder::ReadInt(packet, offset);
        EntityStatus status = (EntityStatus) PacketDecoder::ReadByte(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tStatus: " + StringUtils::to_string(status));
    }

    void PacketReceiver::RecvEntityMetadata(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutEntityMetadata...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        EntityMetaData meta_data = PacketDecoder::ReadMetaData(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tMeta Data: size " + std::to_string(meta_data.GetValues().size()));
    }

    void PacketReceiver::RecvEntityEffect(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutEntityEffect...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        uint8_t effect_id = PacketDecoder::ReadByte(packet, offset);
        uint8_t amplifier = PacketDecoder::ReadByte(packet, offset);
        int duration = PacketDecoder::ReadVarInt(packet, offset);
        bool hide_particles = PacketDecoder::ReadBoolean(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tEffect ID: " + std::to_string((int)effect_id) +
            "\n\tAmplifier: " + std::to_string((int)amplifier) +
            "\n\tDuration: " + std::to_string(duration) +
            "\n\tHide Particles: " + std::to_string(hide_particles));
    }

    void PacketReceiver::RecvExperience(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutExperience...");

        float experience_bar = PacketDecoder::ReadFloat(packet, offset);
        int level = PacketDecoder::ReadVarInt(packet, offset);
        int total_experience = PacketDecoder::ReadVarInt(packet, offset);

        this->bot->LogDebug(
            "Experience Bar: " + std::to_string(experience_bar) +
            "\n\tLevel: " + std::to_string(level) +
            "\n\tTotal Experience: " + std::to_string(total_experience));
    }

    void PacketReceiver::RecvEntityAttributes(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutUpdateAttributes...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        int num_attributes = PacketDecoder::ReadInt(packet, offset);

        std::list<Attribute> attributes;
        for (int i = 0; i < num_attributes; i++)
        {
            attributes.push_back(PacketDecoder::ReadAttribute(packet, offset));
        }

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tAttributes: " + std::to_string(attributes.size()));

    }

    void PacketReceiver::RecvMapChunk(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutMapChunk...");

        int chunk_x = PacketDecoder::ReadInt(packet, offset);
        int chunk_z = PacketDecoder::ReadInt(packet, offset);
        bool ground_up_continuous = PacketDecoder::ReadBoolean(packet, offset);
        uint16_t primary_bitmask = PacketDecoder::ReadShort(packet, offset);
        int data_size = PacketDecoder::ReadVarInt(packet, offset);
        Chunk chunk = PacketDecoder::ReadChunk(chunk_x, chunk_z, ground_up_continuous, true, primary_bitmask, packet, offset);
    
        this->bot->LogDebug(
            "X: " + std::to_string(chunk_x) +
            "\n\tZ: " + std::to_string(chunk_z));

        this->bot->LoadChunk(chunk);
    }

    void PacketReceiver::RecvMultiBlockChange(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutMultiBlockChange...");

        int chunk_x = PacketDecoder::ReadInt(packet, offset);
        int chunk_z = PacketDecoder::ReadInt(packet, offset);
        int record_count = PacketDecoder::ReadVarInt(packet, offset);

        Chunk& chunk = this->bot->GetChunk(chunk_x, chunk_z);
        for (int i = 0; i < record_count; i++)
        {
            uint8_t horizontal_position = PacketDecoder::ReadByte(packet, offset);
            uint8_t y = PacketDecoder::ReadByte(packet, offset);
            int block_id = PacketDecoder::ReadVarInt(packet, offset);

            int x = horizontal_position >> 4 + chunk_x << 4;
            int z = horizontal_position & 0x0F + chunk_z << 4;

            chunk.UpdateBlock(x, y, z, block_id);
        }

        this->bot->LogDebug(
            "X: " + std::to_string(chunk_x) +
            "\n\tZ: " + std::to_string(chunk_z));
    }

    void PacketReceiver::RecvBlockChange(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutBlockChange...");

        Position location = PacketDecoder::ReadPosition(packet, offset);
        int block_id = PacketDecoder::ReadVarInt(packet, offset);

        this->bot->LogDebug(
            "Location: " + location.ToString() +
            "\n\tBlock ID: " + std::to_string(block_id));

        Chunk& chunk = this->bot->GetChunk(location.ToVector());
        chunk.UpdateBlock(location.ToVector(), block_id);
    }

    void PacketReceiver::RecvBlockBreakAnimation(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutBlockBreakAnimation...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        Position location = PacketDecoder::ReadPosition(packet, offset);
        uint8_t destroy_stage = PacketDecoder::ReadByte(packet, offset);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tLocation: " + location.ToString() +
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

    void PacketReceiver::RecvMapChunkBulk(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutMapChunkBulk...");

        bool sky_light_sent = PacketDecoder::ReadBoolean(packet, offset);
        int chunk_column_count = PacketDecoder::ReadVarInt(packet, offset);
        std::list<Chunk> chunks = std::list<Chunk>();

        for (int i = 0; i < chunk_column_count; i++)
        {
            int x = PacketDecoder::ReadInt(packet, offset);
            int z = PacketDecoder::ReadInt(packet, offset);
            uint16_t primary_bit_mask = PacketDecoder::ReadShort(packet, offset);
            chunks.push_back(Chunk(x, z, primary_bit_mask));
            this->bot->LogDebug("Loading Chunk (" + std::to_string(x) + "," + std::to_string(z) + ")");
        }

        for (Chunk chunk : chunks)
        {
            PacketDecoder::ReadChunkBulk(chunk, sky_light_sent, packet, offset);
            this->bot->LoadChunk(chunk);
        }
    }

    void PacketReceiver::RecvWorldEvent(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutWorldEvent...");

        int effect_id = PacketDecoder::ReadInt(packet, offset);
        Position location = PacketDecoder::ReadPosition(packet, offset);
        int data = PacketDecoder::ReadInt(packet, offset);
        bool disable_relative_volume = PacketDecoder::ReadBoolean(packet, offset);

        this->bot->LogDebug(
            "Effect ID: " + std::to_string(effect_id) +
            "\n\tPosition: " + location.ToString());
    }

    void PacketReceiver::RecvNamedSoundEffect(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutNamedSoundEffect...");

        std::string sound_name = PacketDecoder::ReadString(packet, offset);
        Vector<int> position = PacketDecoder::ReadVector<int>(packet, offset);
        float volume = PacketDecoder::ReadFloat(packet, offset);
        uint8_t pitch = PacketDecoder::ReadByte(packet, offset);

        this->bot->LogDebug(
            "Sound Name: " + sound_name +
            "\n\tPosition: " + position.ToString());
    }

    void PacketReceiver::RecvWorldParticles(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutWorldParticles...");

        ParticleType particle_id = (ParticleType) PacketDecoder::ReadInt(packet, length);
        bool long_distance = PacketDecoder::ReadBoolean(packet, length);
        Vector<float> position = PacketDecoder::ReadVector<float>(packet, length);
        Vector<float> offset_vec = PacketDecoder::ReadVector<float>(packet, length);
        float particle_data = PacketDecoder::ReadFloat(packet, length);
        int particle_count = PacketDecoder::ReadInt(packet, length);

        int data_length = 0;
        if (particle_id == ParticleType::ICON_CRACK)
        {
            data_length = 2;
        }
        else if (particle_id == ParticleType::BLOCK_CRACK || particle_id == ParticleType::BLOCK_DUST)
        {
            data_length = 1;
        }

        Buffer<int> data = PacketDecoder::ReadVarIntArray(data_length, packet, offset);

        this->bot->LogDebug(
            "Particle ID: " + StringUtils::to_string(particle_id) +
            "\n\tLong Distance: " + std::to_string(long_distance) +
            "\n\tPosition: " + position.ToString() +
            "\n\tOffset: " + offset_vec.ToString() +
            "\n\tParticle Data: " + std::to_string(particle_data)
        );
    }

    void PacketReceiver::RecvGameStateChange(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutGameStateChange...");

        uint8_t reason = PacketDecoder::ReadByte(packet, offset);
        float value = PacketDecoder::ReadFloat(packet, offset);

        this->bot->LogDebug(
            "Reason: " + std::to_string((int)reason) +
            "\n\tValue: " + std::to_string(value));
    }

    void PacketReceiver::RecvSpawnEntityWeather(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutSpawnEntityWeather...");

        int entity_id = PacketDecoder::ReadVarInt(packet, offset);
        WeatherEntityType type = (WeatherEntityType) PacketDecoder::ReadByte(packet, offset);
        Vector<int> position = PacketDecoder::ReadVector<int>(packet, offset);
        Vector<double> position1 = Vector<double>(position.GetX() / 32.0, position.GetY() / 32.0, position.GetZ() / 32.0);

        this->bot->LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tPosition: " + position1.ToString());
    }

    void PacketReceiver::RecvSetSlot(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutSetSlot...");

        uint8_t window_id = PacketDecoder::ReadByte(packet, offset);
        uint16_t slot_number = PacketDecoder::ReadShort(packet, offset);
        Slot slot = PacketDecoder::ReadSlot(packet, offset);

        this->bot->LogDebug(
            "Window ID: " + std::to_string(window_id) +
            "\n\tSlot Number: " + std::to_string(slot_number) +
            "\n\tSlot: " + slot.ToString()
        );
    }

    void PacketReceiver::RecvWindowItems(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutWindowItems...");

        uint8_t window_id = PacketDecoder::ReadByte(packet, offset);
        int16_t count = PacketDecoder::ReadShort(packet, offset);

        if (window_id == 0)
        {
            std::array<Slot, 45> player_inventory;
            for (int i = 0; i < 45; i++)
            {
                player_inventory[i] = PacketDecoder::ReadSlot(packet, offset);
            }

            this->player->SetInventory(player_inventory);
        }
        else
        {
            std::list<Slot> slots = PacketDecoder::ReadSlotArray(count, packet, offset);
        }

        this->bot->LogDebug(
            "Window ID: " + std::to_string(window_id) +
            "\n\tCount: " + std::to_string(count)
        );
    }

    void PacketReceiver::RecvTransaction(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutTransaction...");

        uint8_t window_id = PacketDecoder::ReadByte(packet, offset);
        int16_t action_number = PacketDecoder::ReadShort(packet, offset);
        bool accepted = PacketDecoder::ReadBoolean(packet, offset);

        this->bot->LogDebug(
            "Window ID: " + std::to_string(window_id) +
            "\n\tAction Number: " + std::to_string(action_number) +
            "\n\tAccept: " + std::to_string(accepted)
        );
    }

    void PacketReceiver::RecvUpdateSign(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutUpdateSign...");

        Position location = PacketDecoder::ReadPosition(packet, offset);
        std::string line1 = PacketDecoder::ReadString(packet, offset);
        std::string line2 = PacketDecoder::ReadString(packet, offset);
        std::string line3 = PacketDecoder::ReadString(packet, offset);
        std::string line4 = PacketDecoder::ReadString(packet, offset);

        this->bot->LogDebug(
            "Location: " + location.ToString() +
            "\n\tLine 1: " + line1 +
            "\n\tLine 2: " + line2 +
            "\n\tLine 3: " + line3 +
            "\n\tLine 4: " + line4
        );
    }

    void PacketReceiver::RecvTileEntityData(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutTileEntityData...");

        Position location = PacketDecoder::ReadPosition(packet, offset);
        TileEntityAction action = (TileEntityAction) PacketDecoder::ReadByte(packet, offset);
        NBTTagCompound nbt = PacketDecoder::ReadNBTTagCompound(packet, offset, true);

        this->bot->LogDebug(
            "Location: " + location.ToString() +
            "\n\tAction: " + StringUtils::to_string(action) +
            "\n\tNBT: " + nbt.ToString()
        );
    }

    void PacketReceiver::RecvStatistics(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutStatistics...");
        int count = PacketDecoder::ReadVarInt(packet, offset);
        std::list<Statistic> statistics = PacketDecoder::ReadStatisticArray(count, packet, offset);

        this->bot->LogDebug("Statistics (" + std::to_string(count) + "): ");

        for (auto statistic : statistics)
        {
            this->bot->LogDebug("\t" + statistic.ToString());
        }
    }

    void PacketReceiver::RecvPlayerInfo(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutPlayerInfo...");

        PlayerInfoAction action = (PlayerInfoAction) PacketDecoder::ReadVarInt(packet, offset);
        int players_length = PacketDecoder::ReadVarInt(packet, offset);
        this->bot->UpdatePlayerInfo(action, players_length, packet, offset);
    }

    void PacketReceiver::RecvAbilities(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutAbilities...");
        uint8_t flags = PacketDecoder::ReadByte(packet, offset);
        float flying_speed = PacketDecoder::ReadFloat(packet, offset);
        float fov_modifier = PacketDecoder::ReadFloat(packet, offset);

        this->bot->LogDebug(
            "Flags: " + std::to_string((int)flags) +
            "\n\tFlying Speed: " + std::to_string(flying_speed) +
            "\n\tFOV Modifier: " + std::to_string(fov_modifier));
    }

    void PacketReceiver::RecvScoreboardObjective(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutScoreboardObjective...");
        std::string objective_name = PacketDecoder::ReadString(packet, offset);
        ScoreboardMode mode = (ScoreboardMode) PacketDecoder::ReadByte(packet, offset);

        this->bot->LogDebug(
            "Objective Name: " + objective_name +
            "\n\tMode: " + StringUtils::to_string(mode));

        if (mode == ScoreboardMode::CREATE ||
            mode == ScoreboardMode::UPDATE)
        {
            std::string objective_value = PacketDecoder::ReadString(packet, offset);
            std::string objective_type = PacketDecoder::ReadString(packet, offset);
            this->bot->LogDebug(
                "Objective value: " + objective_value +
                "\n\tObjective type: " + objective_type);
        }
    }

    void PacketReceiver::RecvUpdateScoreboardScore(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutScoreboardScore...");
        std::string score_name = PacketDecoder::ReadString(packet, offset);
        ScoreAction action = (ScoreAction) PacketDecoder::ReadByte(packet, offset);
        std::string objective_name = PacketDecoder::ReadString(packet, offset);

        this->bot->LogDebug(
            "Score Name: " + score_name +
            "\n\tAction: " + StringUtils::to_string(action) +
            "\n\tObjective Name: " + objective_name);

        if (action != ScoreAction::REMOVE)
        {
            int value = PacketDecoder::ReadVarInt(packet, offset);
            this->bot->LogDebug("\n\tValue: " + std::to_string(value));
        }

    }

    void PacketReceiver::RecvDisplayScoreboard(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutScoreboardDisplayObjective...");

        ScoreboardPosition position = (ScoreboardPosition) PacketDecoder::ReadByte(packet, offset);
        std::string score_name = PacketDecoder::ReadString(packet, offset);

        this->bot->LogDebug(
            "Position: " + StringUtils::to_string(position) +
            "\n\tScore Name: " + score_name);
    }

    void PacketReceiver::RecvScoreboardTeam(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutScoreboardTeam...");
        std::string team_name = PacketDecoder::ReadString(packet, offset);
        ScoreboardMode mode = (ScoreboardMode) PacketDecoder::ReadByte(packet, offset);

        this->bot->LogDebug(
            "Team Name: " + team_name +
            "\n\tMode: " + StringUtils::to_string(mode));

        // Creating or updating team
        if (mode == ScoreboardMode::CREATE ||
            mode == ScoreboardMode::UPDATE)
        {
            std::string team_display_name = PacketDecoder::ReadString(packet, offset);
            std::string team_prefix = PacketDecoder::ReadString(packet, offset);
            std::string team_suffix = PacketDecoder::ReadString(packet, offset);

            uint8_t friendly = PacketDecoder::ReadByte(packet, offset);
            this->bot->LogDebug("Friendly: " + std::to_string((int)friendly));
            FriendlyFire friendly_fire = (FriendlyFire) friendly;
            std::string nametag_visibility = PacketDecoder::ReadString(packet, offset);
            uint8_t color = PacketDecoder::ReadByte(packet, offset);

            this->bot->LogDebug(
                "Team Display Name: " + team_display_name +
                "\n\tTeam Prefix: " + team_prefix +
                "\n\tTeam Suffix: " + team_suffix +
                "\n\tFriendly Fire: " + StringUtils::to_string(friendly_fire) +
                "\n\tNametag Visibility: " + nametag_visibility +
                "\n\tColor: " + std::to_string((int)color));
        }

        // Creating or updating players on team
        if (mode == ScoreboardMode::CREATE ||
            mode == ScoreboardMode::PLAYER_ADDED ||
            mode == ScoreboardMode::PLAYER_REMOVED)
        {
            int player_count = PacketDecoder::ReadVarInt(packet, offset);
            std::list<std::string> players = PacketDecoder::ReadStringArray(player_count, packet, offset);

            this->bot->LogDebug("Players (" + std::to_string(player_count) + "): ");

            for (auto player : players)
            {
                this->bot->LogDebug("\t\t" + player);
            }
        }

    }

    void PacketReceiver::RecvPluginMessage(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutCustomPayload...");

        std::string plugin_channel = PacketDecoder::ReadString(packet, offset);
        Buffer<uint8_t> data = PacketDecoder::ReadByteArray(length - offset, packet, offset);

        this->bot->LogDebug(
            "Plugin Channel: " + plugin_channel +
            "\n\tData: " + data.to_string());
    }

    void PacketReceiver::RecvServerDifficulty(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutServerDifficulty...");

        Difficulty difficulty = (Difficulty) PacketDecoder::ReadByte(packet, offset);

        this->bot->LogDebug("Difficulty: " + StringUtils::to_string(difficulty));
    }

    void PacketReceiver::RecvWorldBorder(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutWorldBorder...");

        WorldBorderAction action = (WorldBorderAction) PacketDecoder::ReadVarInt(packet, offset);

        switch (action)
        {
        case WorldBorderAction::SET_SIZE:
        {
            double radius = PacketDecoder::ReadDouble(packet, offset);
            //this->world_border.set_radius(radius);
            break;
        }

        case WorldBorderAction::LERP_SIZE:
        {
            double old_radius = PacketDecoder::ReadDouble(packet, offset);
            double new_radius = PacketDecoder::ReadDouble(packet, offset);
            long speed = PacketDecoder::ReadVarLong(packet, offset);
            //this->world_border.set_old_radius(old_radius);
            //this->world_border.set_new_radius(new_radius);
            //this->world_border.set_speed(speed);
            break;
        }

        case WorldBorderAction::SET_CENTER:
        {
            double x = PacketDecoder::ReadDouble(packet, offset);
            double z = PacketDecoder::ReadDouble(packet, offset);
            //this->world_border.set_x(x);
            //this->world_border.set_z(z);
            break;
        }

        case WorldBorderAction::INITIALIZE:
        {
            double x = PacketDecoder::ReadDouble(packet, offset);
            double z = PacketDecoder::ReadDouble(packet, offset);
            double old_radius = PacketDecoder::ReadDouble(packet, offset);
            double new_radius = PacketDecoder::ReadDouble(packet, offset);
            long speed = PacketDecoder::ReadVarLong(packet, offset);
            int portal_teleport_boundary = PacketDecoder::ReadVarInt(packet, offset);
            int warning_time = PacketDecoder::ReadVarInt(packet, offset);
            int warning_blocks = PacketDecoder::ReadVarInt(packet, offset);

            //this->world_border.set_x(x);
            //this->world_border.set_z(z);
            //this->world_border.set_old_radius(old_radius);
            //this->world_border.set_new_radius(new_radius);
            //this->world_border.set_speed(speed);
            //this->world_border.set_portal_teleport_boundary(portal_teleport_boundary);
            //this->world_border.set_warning_time(warning_time);
            //this->world_border.set_warning_blocks(warning_blocks);
            break;
        }

        case WorldBorderAction::SET_WARNING_TIME:
        {
            int warning_time = PacketDecoder::ReadVarInt(packet, offset);
            //this->world_border.set_warning_time(warning_time);
            break;
        }

        case WorldBorderAction::SET_WARNING_BLOCKS:
        {
            int warning_blocks = PacketDecoder::ReadVarInt(packet, offset);
            //this->world_border.set_warning_blocks(warning_blocks);
            break;
        }

        }

    }

    void PacketReceiver::RecvTitle(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutTitle...");

        TitleAction action = (TitleAction) PacketDecoder::ReadVarInt(packet, offset);

        switch (action)
        {
        case TitleAction::SET_TITLE:
        {
            std::string title_text = PacketDecoder::ReadString(packet, offset);
            break;
        }
        case TitleAction::SET_SUBTITLE:
        {
            std::string subtitle_text = PacketDecoder::ReadString(packet, offset);
            break;
        }
        case TitleAction::SET_TIMES_AND_DISPLAY:
        {
            int fade_in = PacketDecoder::ReadInt(packet, offset);
            int stay = PacketDecoder::ReadInt(packet, offset);
            int fade_out = PacketDecoder::ReadInt(packet, offset);
            break;
        }

        }

        this->bot->LogDebug("Action: " + StringUtils::to_string(action));
    }

    void PacketReceiver::RecvPlayerListHeaderFooter(uint8_t* packet, size_t length, size_t& offset)
    {
        this->bot->LogDebug("<<< Handling PacketPlayOutPlayerListHeaderFooter...");

        std::string header = PacketDecoder::ReadString(packet, offset);
        std::string footer = PacketDecoder::ReadString(packet, offset);

        this->bot->LogDebug(
            "Header: " + header +
            "\n\tFooter: " + footer);
    }
}
