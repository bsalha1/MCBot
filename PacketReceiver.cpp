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
            size_t bytes_read = this->bot->GetSocket().RecvPacket(packet, 1);
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
        this->bot->GetLogger().LogDebug("Length: " + std::to_string(length));

        int decompressed_length = 0;
        if (this->compression_enabled)
        {
            decompressed_length = this->ReadNextVarInt();
            length -= PacketEncoder::GetVarIntNumBytes(decompressed_length);
            this->bot->GetLogger().LogDebug("Decompressed length: " + std::to_string(decompressed_length));
        }

        Packet packet = this->ReadNextPacket(length, decompressed_length);
        if (packet.data == NULL)
        {
            this->bot->GetLogger().LogError("Invalid packet received");
            return;
        }

        int packet_id = PacketDecoder::ReadVarInt(packet);
        this->bot->GetLogger().LogDebug("ID: " + std::to_string(packet_id));

        HandleRecvPacket(packet_id, packet);

        delete[] packet.data;
    }

    Packet PacketReceiver::ReadNextPacket(int length, int decompressed_length)
    {
        uint8_t* data = new uint8_t[decompressed_length == 0 ? length : decompressed_length]{ 0 };

        int bytes_read = this->bot->GetSocket().RecvPacket(data, length, decompressed_length);
        if (bytes_read < 0)
        {
            this->bot->GetLogger().LogError("Failed to receive packet");
            delete[] data;
            return Packet(bytes_read);
        }
        this->bot->GetLogger().LogDebug("Received Packet: " + std::to_string(bytes_read) + "bytes");

        Packet packet = Packet(bytes_read);
        packet.data = data;
        return packet;
    }

    void PacketReceiver::HandleRecvPacket(int packet_id, Packet packet)
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
                this->RecvLoginDisconnect(packet);
                break;
            case 0x01:
                this->RecvEncryptionRequest(packet);
                break;
            case 0x02:
                this->RecvLoginSuccess(packet);
                break;
            case 0x03:
                this->RecvSetCompression(packet);
                break;
            default:
                this->bot->GetLogger().LogError("Unhandled " + StringUtils::to_string(this->bot->GetState()) + " packet id: " + std::to_string(packet_id));
            }
        }
        else if (this->bot->GetState() == State::PLAY)
        {
            switch (packet_id)
            {
            case 0x00:
                this->RecvKeepAlive(packet);
                break;
            case 0x01:
                this->RecvJoinServer(packet);
                break;
            case 0x02:
                this->RecvChatMessage(packet);
                break;
            case 0x03:
                this->RecvUpdateTime(packet);
                break;
            case 0x04:
                this->RecvEntityEquipment(packet);
                break;
            case 0x05:
                this->RecvSpawnPosition(packet);
                break;
            case 0x06:
                this->RecvUpdateHealth(packet);
                break;
            case 0x08:
                this->RecvPosition(packet);
                break;
            case 0x09:
                this->RecvHeldItemSlot(packet);
                break;
            case 0x0A:
                this->RecvBed(packet);
                break;
            case 0x0B:
                this->RecvAnimation(packet);
                break;
            case 0x0C:
                this->RecvNamedEntitySpawn(packet);
                break;
            case 0x0D:
                this->RecvCollect(packet);
                break;
            case 0x0E:
                this->RecvSpawnEntity(packet);
                break;
            case 0x0F:
                this->RecvSpawnEntityLiving(packet);
                break;
            case 0x10:
                this->RecvSpawnEntityPainting(packet);
                break;
            case 0x11:
                this->RecvSpawnEntityExperienceOrb(packet);
                break;
            case 0x12:
                this->RecvEntityVelocity(packet);
                break;
            case 0x13:
                this->RecvEntityDestroy(packet);
                break;
            case 0x14:
                this->RecvEntity(packet);
                break;
            case 0x15:
                this->RecvRelEntityMove(packet);
                break;
            case 0x16:
                this->RecvEntityLook(packet);
                break;
            case 0x17:
                this->RecvRelEntityMoveLook(packet);
                break;
            case 0x18:
                this->RecvEntityTeleport(packet);
                break;
            case 0x19:
                this->RecvEntityHeadLook(packet);
                break;
            case 0x1A:
                this->RecvEntityStatus(packet);
                break;
            case 0x1C:
                this->RecvEntityMetadata(packet);
                break;
            case 0x1D:
                this->RecvEntityEffect(packet);
                break;
            case 0x1F:
                this->RecvExperience(packet);
                break;
            case 0x20:
                this->RecvEntityAttributes(packet);
                break;
            case 0x21:
                this->RecvMapChunk(packet);
                break;
            case 0x22:
                this->RecvMultiBlockChange(packet);
                break;
            case 0x23:
                this->RecvBlockChange(packet);
                break;
            case 0x25:
                this->RecvBlockBreakAnimation(packet);
                break;
            case 0x28:
                this->RecvWorldEvent(packet);
                break;
            case 0x26:
                this->RecvMapChunkBulk(packet);
                break;
            case 0x29:
                this->RecvNamedSoundEffect(packet);
                break;
            case 0x2A:
                this->RecvWorldParticles(packet);
                break;
            case 0x2B:
                this->RecvGameStateChange(packet);
                break;
            case 0x2C:
                this->RecvSpawnEntityWeather(packet);
                break;
            case 0x2F:
                this->RecvSetSlot(packet);
                break;
            case 0x30:
                this->RecvWindowItems(packet);
                break;
            case 0x32:
                this->RecvTransaction(packet);
                break;
            case 0x33:
                this->RecvUpdateSign(packet);
                break;
            case 0x35:
                this->RecvTileEntityData(packet);
                break;
            case 0x37:
                this->RecvStatistics(packet);
                break;
            case 0x38:
                this->RecvPlayerInfo(packet);
                break;
            case 0x39:
                this->RecvAbilities(packet);
                break;
            case 0x3B:
                this->RecvScoreboardObjective(packet);
                break;
            case 0x3C:
                this->RecvUpdateScoreboardScore(packet);
                break;
            case 0x3D:
                this->RecvDisplayScoreboard(packet);
                break;
            case 0x3E:
                this->RecvScoreboardTeam(packet);
                break;
            case 0x3F:
                this->RecvPluginMessage(packet);
                break;
            case 0x40:
                this->RecvPlayDisconnect(packet);
                break;
            case 0x41:
                this->RecvServerDifficulty(packet);
                break;
            case 0x44:
                this->RecvWorldBorder(packet);
                break;
            case 0x45:
                this->RecvTitle(packet);
                break;
            case 0x47:
                this->RecvPlayerListHeaderFooter(packet);
                break;
            default:
                this->bot->GetLogger().LogError("Unhandled " + StringUtils::to_string(this->bot->GetState()) + " packet id: " + std::to_string(packet_id));
            }
        }
        else
        {
            this->bot->GetLogger().LogError("Unknown state: " + StringUtils::to_string(this->bot->GetState()));
        }
    }

    void PacketReceiver::RecvLoginDisconnect(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketLoginOutDisconnect...");

        std::cout << packet.data << std::endl;

        this->bot->SetConnected(false);
        this->bot->GetLogger().LogInfo("Disconnected");
    }

    void PacketReceiver::RecvSetCompression(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketLoginOutSetCompression...");
        int max_uncompressed_length = PacketDecoder::ReadVarInt(packet);

        this->bot->GetLogger().LogDebug("\tMax Uncompressed Length: " + std::to_string(max_uncompressed_length));
        this->compression_enabled = true;
        this->bot->GetSocket().InitCompression(max_uncompressed_length);
        this->bot->GetLogger().LogDebug("<<< COMPRESSION ENABLED <<<");
    }

    void PacketReceiver::RecvEncryptionRequest(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketLoginOutEncryptionRequest...");

        // Server ID //
        std::string server_id = PacketDecoder::ReadString(packet);

        // Public Key //
        int public_key_length = PacketDecoder::ReadVarInt(packet);
        uint8_t* public_key = new uint8_t[public_key_length];
        PacketDecoder::ReadByteArray(public_key, public_key_length, packet);

        // Verify Token //
        int verify_token_length = PacketDecoder::ReadVarInt(packet);
        uint8_t* verify_token = new uint8_t[verify_token_length];
        PacketDecoder::ReadByteArray(verify_token, verify_token_length, packet);

        std::string shared_secret = GetRandomHexBytes(16);

        // Save Session //
        // - So Yggdrasil authentication doesn't kick us!
        this->bot->GetLogger().LogDebug("Saving session...");
        if (this->bot->GetPacketSender().SendSession(server_id, shared_secret, public_key, public_key_length) < 0)
        {
            this->bot->GetLogger().LogError("Invalid session!");
        }
        else
        {
            this->bot->GetPacketSender().SendEncryptionResponse(public_key, public_key_length, verify_token, verify_token_length, shared_secret);
        }
    }

    void PacketReceiver::RecvLoginSuccess(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayLoginOutSuccess...");

        std::string uuid_string = PacketDecoder::ReadString(packet);
        std::string username = PacketDecoder::ReadString(packet);

        this->bot->GetLogger().LogDebug(
            "UUID: " + uuid_string + '\n' +
            "\tUsername: " + username);

        this->bot->GetLogger().LogInfo("Successfully logged in!");

        this->bot->SetState(State::PLAY);
    }

    void PacketReceiver::RecvPlayDisconnect(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutKickDisconnect...");

        std::string reason = PacketDecoder::ReadString(packet);

        this->bot->GetLogger().LogInfo("Disconnected: " + reason);

        this->bot->SetConnected(false);
    }

    void PacketReceiver::RecvKeepAlive(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutKeepAlive...");

        int id = PacketDecoder::ReadVarInt(packet);

        this->bot->GetPacketSender().SendKeepAlive(id);
    }

    void PacketReceiver::RecvJoinServer(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutJoinServer...");

        int entity_id = PacketDecoder::ReadInt(packet);
        Gamemode gamemode = (Gamemode) PacketDecoder::ReadByte(packet);
        Dimension dimension = (Dimension) PacketDecoder::ReadByte(packet);
        Difficulty difficulty = (Difficulty) PacketDecoder::ReadByte(packet);
        uint8_t max_players = PacketDecoder::ReadByte(packet);
        std::string level_type = PacketDecoder::ReadString(packet);
        bool reduced_debug_info = PacketDecoder::ReadBoolean(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tGamemode: " + StringUtils::to_string(gamemode) +
            "\n\tDimension: " + StringUtils::to_string(dimension) +
            "\n\tDifficulty: " + StringUtils::to_string(difficulty) +
            "\n\tMax Players: " + std::to_string((int)max_players) +
            "\n\tLevel Type: " + level_type +
            "\n\tReduced Debug Info: " + std::to_string(reduced_debug_info));


    }

    void PacketReceiver::RecvChatMessage(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutChat...");

        std::string chat_data = PacketDecoder::ReadString(packet);
        ChatPosition position = (ChatPosition) PacketDecoder::ReadByte(packet);

        this->bot->GetLogger().LogDebug(
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

        this->bot->GetLogger().LogChat(chat_message);
    }

    void PacketReceiver::RecvUpdateTime(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutUpdateTime...");

        int64_t world_age = (int64_t)PacketDecoder::ReadLong(packet);
        int64_t time_of_day = (int64_t)PacketDecoder::ReadLong(packet);

        this->bot->GetLogger().LogDebug(
            "World Age: " + std::to_string(world_age) +
            "\n\tTime of Day: " + std::to_string(time_of_day));
    }

    void PacketReceiver::RecvEntityEquipment(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutEntityEquipment...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        short slot = PacketDecoder::ReadShort(packet);
        Slot item = PacketDecoder::ReadSlot(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tSlot: " + std::to_string(slot) +
            "\n\tItem: " + item.ToString());
    }

    void PacketReceiver::RecvSpawnPosition(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutSpawnPosition...");

        Position location = PacketDecoder::ReadPosition(packet);

        this->bot->GetLogger().LogDebug("Location: " + location.ToString());
    }

    void PacketReceiver::RecvUpdateHealth(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutUpdateHealth...");

        float health = PacketDecoder::ReadFloat(packet);
        int food = PacketDecoder::ReadVarInt(packet);
        float food_saturation = PacketDecoder::ReadFloat(packet);

        this->bot->GetLogger().LogDebug("Health: " + std::to_string(health) +
            "\n\tFood: " + std::to_string(food) +
            "\n\tSaturation: " + std::to_string(food_saturation));
    }


    void PacketReceiver::RecvPosition(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutPosition...");

        Vector<double> position = PacketDecoder::ReadVector<double>(packet);
        float yaw = PacketDecoder::ReadFloat(packet);
        float pitch = PacketDecoder::ReadFloat(packet);
        uint8_t flags = PacketDecoder::ReadByte(packet);

        this->bot->GetLogger().LogDebug("Position: " + position.ToString() +
            "\n\tYaw: " + std::to_string(yaw) +
            "\n\tPitch: " + std::to_string(pitch) +
            "\n\tFlags: " + std::to_string((int)flags));

        this->bot->GetPlayer().UpdateLocation(position);
        this->bot->GetPlayer().UpdateRotation(yaw, pitch);
    }

    void PacketReceiver::RecvHeldItemSlot(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutHeldItemSlot...");

        uint8_t held_item_slot = PacketDecoder::ReadByte(packet);

        this->bot->GetLogger().LogDebug("Held Item Slot: " + std::to_string((int)held_item_slot));
    }

    void PacketReceiver::RecvBed(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutBed...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        Position position = PacketDecoder::ReadPosition(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tLocation: " + position.ToString());
    }

    void PacketReceiver::RecvAnimation(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutAnimation...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        Position position = PacketDecoder::ReadPosition(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tLocation: " + position.ToString());
    }

    void PacketReceiver::RecvNamedEntitySpawn(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutNamedEntitySpawn...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        UUID uuid = PacketDecoder::ReadUUID(packet);
        int x = PacketDecoder::ReadInt(packet);
        int y = PacketDecoder::ReadInt(packet);
        int z = PacketDecoder::ReadInt(packet);
        Vector<double> position = Vector<double>(x / 32.0, y / 32.0, z / 32.0);

        uint8_t pitch = PacketDecoder::ReadByte(packet);
        uint8_t yaw = PacketDecoder::ReadByte(packet);
        short current_item = PacketDecoder::ReadShort(packet);
        EntityMetaData meta_data = PacketDecoder::ReadMetaData(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tUUID: " + uuid.ToString() +
            "\n\tLocation: " + position.ToString());

        auto& player_registry = this->bot->GetPlayerRegistry();
        auto& entity_registry = this->bot->GetEntityRegistry();
        if (!player_registry.IsValueRegistered(uuid))
        {
            EntityPlayer player = EntityPlayer(entity_id, uuid);
            player.UpdateLocation(position);
            player.UpdateRotation(yaw, pitch);
            player_registry.RegisterValue(player.GetUUID(), player);
            entity_registry.RegisterValue(player.GetID(), player);
        }
        else
        {
            EntityPlayer& player = player_registry.GetValue(uuid);
            player.SetID(entity_id);
            player.UpdateLocation(position);
            player.UpdateRotation(yaw, pitch);
            entity_registry.RegisterValue(player.GetID(), player);
        }
    }

    void PacketReceiver::RecvCollect(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutCollect...");

        int collected_id = PacketDecoder::ReadVarInt(packet);
        int collector_id = PacketDecoder::ReadVarInt(packet);

        this->bot->GetLogger().LogDebug(
            "Collected ID: " + std::to_string(collected_id) +
            "\n\tCollector ID: " + std::to_string(collector_id));
    }

    void PacketReceiver::RecvSpawnEntity(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutSpawnEntity...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        EntityType type = (EntityType)PacketDecoder::ReadByte(packet);
        Vector<int> position = PacketDecoder::ReadVector<int>(packet);
        Vector<double> position1 = Vector<double>(position.GetX() / 32.0, position.GetY() / 32.0, position.GetZ() / 32.0);

        uint8_t pitch = PacketDecoder::ReadByte(packet);
        uint8_t yaw = PacketDecoder::ReadByte(packet);
        int data = PacketDecoder::ReadInt(packet);

        if (data > 0)
        {
            Vector<short> motion = PacketDecoder::ReadVector<short>(packet);
        }

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tLocation:" + position.ToString());

        auto& entity_registry = this->bot->GetEntityRegistry();
        if (entity_registry.IsValueRegistered(entity_id))
        {
            entity_registry.GetValue(entity_id).UpdateLocation(position1);
        }
        else
        {
            Entity entity = Entity(type, entity_id);
            entity.UpdateLocation(position1);
            entity_registry.RegisterValue(entity_id, entity);
        }
    }

    void PacketReceiver::RecvSpawnEntityLiving(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutSpawnEntityLiving...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        EntityType type = (EntityType)PacketDecoder::ReadByte(packet);
        Vector<int> position = PacketDecoder::ReadVector<int>(packet);
        Vector<double> position1 = Vector<double>(position.GetX() / 32.0, position.GetY() / 32.0, position.GetZ() / 32.0);

        uint8_t pitch = PacketDecoder::ReadByte(packet);
        uint8_t yaw = PacketDecoder::ReadByte(packet);
        uint8_t head_pitch = PacketDecoder::ReadByte(packet);
        Vector<short> motion = PacketDecoder::ReadVector<short>(packet);
        EntityMetaData meta_data = PacketDecoder::ReadMetaData(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tLocation:" + position.ToString());

        auto& entity_registry = this->bot->GetEntityRegistry();
        if (entity_registry.IsValueRegistered(entity_id))
        {
            entity_registry.GetValue(entity_id).UpdateLocation(position1);
        }
        else
        {
            EntityLiving entity = EntityLiving(type, entity_id);
            entity.UpdateLocation(position1);
            entity_registry.RegisterValue(entity_id, entity);
        }
    }

    void PacketReceiver::RecvSpawnEntityPainting(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutSpawnEntityPainting...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        std::string title = PacketDecoder::ReadString(packet);
        Position location = PacketDecoder::ReadPosition(packet);
        uint8_t direction = PacketDecoder::ReadByte(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tLocation: " + location.ToString() +
            "\n\tTitle: " + title +
            "\n\tDirection: " + std::to_string((int)direction));

        this->bot->GetEntityRegistry().RegisterValue(entity_id, Entity(EntityType::PAINTING, entity_id));
    }

    void PacketReceiver::RecvSpawnEntityExperienceOrb(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutSpawnEntityExperienceOrb...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        Vector<int> motion = PacketDecoder::ReadVector<int>(packet);
        short count = PacketDecoder::ReadShort(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tPosition: " + motion.ToString() +
            "\n\tCount: " + std::to_string(count));

        this->bot->GetEntityRegistry().RegisterValue(entity_id, Entity(EntityType::EXPERIENCE_ORB, entity_id));
    }

    void PacketReceiver::RecvEntityVelocity(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutEntityVelocity...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        Vector<short> motion = PacketDecoder::ReadVector<short>(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tVelocity: " + motion.ToString());
    }

    void PacketReceiver::RecvEntityDestroy(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutEntityDestroy...");

        int count = PacketDecoder::ReadVarInt(packet);

        std::list<int> entity_ids = std::list <int>();
        for (int i = 0; i < count; i++)
        {
            entity_ids.push_back(PacketDecoder::ReadVarInt(packet));
        }

        this->bot->GetLogger().LogDebug(
            "Entity Count: " + std::to_string(count));

        auto& entity_registry = this->bot->GetEntityRegistry();
        for (int id : entity_ids)
        {
            Entity& entity = entity_registry.GetValue(id);
            entity.Die();
            entity_registry.RemoveValue(id);

            if (entity.GetEntityType() == EntityType::PLAYER)
            {
                this->bot->GetPlayerRegistry().RemoveValue(static_cast<EntityPlayer&>(entity).GetUUID());
            }
        }
    }

    void PacketReceiver::RecvEntity(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutEntity...");

        int entity_id = PacketDecoder::ReadVarInt(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id));

        this->bot->GetEntityRegistry().RegisterValue(entity_id, Entity(entity_id));
    }

    void PacketReceiver::RecvRelEntityMove(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutRelEntityMove...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        Vector<int8_t> dmot = PacketDecoder::ReadVector<int8_t>(packet);
        bool on_ground = PacketDecoder::ReadBoolean(packet);

        Vector<double> dr = Vector<double>(
            dmot.GetX() / 32.0, dmot.GetY() / 32.0, dmot.GetZ() / 32.0);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tChange in Motion: " + dr.ToString() +
            "\n\tOn Ground: " + std::to_string(on_ground));

        this->bot->GetEntityRegistry().GetValue(entity_id).UpdateMotion(dr);
    }

    void PacketReceiver::RecvEntityLook(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutEntityLook...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        uint8_t yaw = PacketDecoder::ReadByte(packet);
        uint8_t pitch = PacketDecoder::ReadByte(packet);
        bool on_ground = PacketDecoder::ReadBoolean(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tYaw: " + std::to_string((int)yaw) +
            "\n\tPitch: " + std::to_string((int)pitch) +
            "\n\tOn Ground: " + std::to_string(on_ground));

        this->bot->GetEntityRegistry().GetValue(entity_id).UpdateRotation(yaw, pitch);
    }

    void PacketReceiver::RecvRelEntityMoveLook(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutRelEntityMoveLook...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        Vector<int8_t> dmot = PacketDecoder::ReadVector<int8_t>(packet);
        Vector<double> dr = Vector<double>(dmot.GetX() / 32.0, dmot.GetY() / 32.0, dmot.GetZ() / 32.0);
        double yaw = PacketDecoder::ReadByte(packet) * 2 * 3.14159 / 256;
        uint8_t pitch = PacketDecoder::ReadByte(packet) * 2 * 3.14159 / 256;
        bool on_ground = PacketDecoder::ReadBoolean(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tChange in Motion: " + dr.ToString() +
            "\n\tYaw: " + std::to_string(yaw) +
            "\n\tPitch: " + std::to_string(pitch) +
            "\n\tOn Ground: " + std::to_string(on_ground));

        auto& entity = this->bot->GetEntityRegistry().GetValue(entity_id);
        entity.UpdateMotion(dr);
        entity.UpdateRotation(yaw, pitch);
    }

    void PacketReceiver::RecvEntityTeleport(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutEntityTeleport...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        Vector<int> position = PacketDecoder::ReadVector<int>(packet);
        Vector<double> position1 = Vector<double>(position.GetX() / 32.0, position.GetY() / 32.0, position.GetZ() / 32.0);

        double yaw = PacketDecoder::ReadByte(packet) * 2 * 3.14159 / 256;
        double pitch = PacketDecoder::ReadByte(packet) * 2 * 3.14159 / 256;
        bool on_ground = PacketDecoder::ReadBoolean(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tChange in Motion: " + position.ToString() +
            "\n\tYaw: " + std::to_string(yaw) +
            "\n\tPitch: " + std::to_string(pitch) +
            "\n\tOn Ground: " + std::to_string(on_ground));

        auto& entity = this->bot->GetEntityRegistry().GetValue(entity_id);
        entity.UpdateLocation(position1);
        entity.UpdateRotation(yaw, pitch);
    }

    void PacketReceiver::RecvEntityHeadLook(Packet packet)
    {
        // TODO: find actual name of packet
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutEntityHeadLook...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        double yaw = PacketDecoder::ReadByte(packet) * 2 * 3.14159 / 256;

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tAngle: " + std::to_string(yaw));

        this->bot->GetEntityRegistry().GetValue(entity_id).UpdateYaw(yaw);
    }

    void PacketReceiver::RecvEntityStatus(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutEntityStatus...");

        int entity_id = PacketDecoder::ReadInt(packet);
        EntityStatus status = (EntityStatus) PacketDecoder::ReadByte(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tStatus: " + StringUtils::to_string(status));
    }

    void PacketReceiver::RecvEntityMetadata(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutEntityMetadata...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        EntityMetaData meta_data = PacketDecoder::ReadMetaData(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tMeta Data: size " + std::to_string(meta_data.GetValues().size()));
    }

    void PacketReceiver::RecvEntityEffect(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutEntityEffect...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        uint8_t effect_id = PacketDecoder::ReadByte(packet);
        uint8_t amplifier = PacketDecoder::ReadByte(packet);
        int duration = PacketDecoder::ReadVarInt(packet);
        bool hide_particles = PacketDecoder::ReadBoolean(packet);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tEffect ID: " + std::to_string((int)effect_id) +
            "\n\tAmplifier: " + std::to_string((int)amplifier) +
            "\n\tDuration: " + std::to_string(duration) +
            "\n\tHide Particles: " + std::to_string(hide_particles));
    }

    void PacketReceiver::RecvExperience(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutExperience...");

        float experience_bar = PacketDecoder::ReadFloat(packet);
        int level = PacketDecoder::ReadVarInt(packet);
        int total_experience = PacketDecoder::ReadVarInt(packet);

        this->bot->GetLogger().LogDebug(
            "Experience Bar: " + std::to_string(experience_bar) +
            "\n\tLevel: " + std::to_string(level) +
            "\n\tTotal Experience: " + std::to_string(total_experience));
    }

    void PacketReceiver::RecvEntityAttributes(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutUpdateAttributes...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        int num_attributes = PacketDecoder::ReadInt(packet);

        std::list<Attribute> attributes;
        for (int i = 0; i < num_attributes; i++)
        {
            attributes.push_back(PacketDecoder::ReadAttribute(packet));
        }

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tAttributes: " + std::to_string(attributes.size()));

    }

    void PacketReceiver::RecvMapChunk(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutMapChunk...");

        int chunk_x = PacketDecoder::ReadInt(packet);
        int chunk_z = PacketDecoder::ReadInt(packet);
        bool ground_up_continuous = PacketDecoder::ReadBoolean(packet);
        uint16_t primary_bitmask = PacketDecoder::ReadShort(packet);
        int data_size = PacketDecoder::ReadVarInt(packet);
        Chunk chunk = PacketDecoder::ReadChunk(chunk_x, chunk_z, ground_up_continuous, true, primary_bitmask, packet);
    
        this->bot->GetLogger().LogDebug(
            "X: " + std::to_string(chunk_x) +
            "\n\tZ: " + std::to_string(chunk_z));

        this->bot->GetChunkRegistry().RegisterValue(std::pair<int, int>(chunk.GetX(), chunk.GetZ()), chunk);
    }

    void PacketReceiver::RecvMultiBlockChange(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutMultiBlockChange...");

        int chunk_x = PacketDecoder::ReadInt(packet);
        int chunk_z = PacketDecoder::ReadInt(packet);
        int record_count = PacketDecoder::ReadVarInt(packet);

        Chunk& chunk = this->bot->GetChunkRegistry().GetValue(std::pair<int, int>(chunk_x, chunk_z));
        for (int i = 0; i < record_count; i++)
        {
            uint8_t horizontal_position = PacketDecoder::ReadByte(packet);
            uint8_t y = PacketDecoder::ReadByte(packet);
            int block_id = PacketDecoder::ReadVarInt(packet);

            int x = horizontal_position >> 4 + chunk_x << 4;
            int z = horizontal_position & 0x0F + chunk_z << 4;

            chunk.UpdateBlock(x, y, z, block_id);
        }

        this->bot->GetLogger().LogDebug(
            "X: " + std::to_string(chunk_x) +
            "\n\tZ: " + std::to_string(chunk_z));
    }

    void PacketReceiver::RecvBlockChange(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutBlockChange...");

        Position location = PacketDecoder::ReadPosition(packet);
        int block_id = PacketDecoder::ReadVarInt(packet);

        this->bot->GetLogger().LogDebug(
            "Location: " + location.ToString() +
            "\n\tBlock ID: " + std::to_string(block_id));

        Chunk& chunk = this->bot->GetChunk(location.ToVector());
        chunk.UpdateBlock(location.ToVector(), block_id);
    }

    void PacketReceiver::RecvBlockBreakAnimation(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutBlockBreakAnimation...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        Position location = PacketDecoder::ReadPosition(packet);
        uint8_t destroy_stage = PacketDecoder::ReadByte(packet);

        this->bot->GetLogger().LogDebug(
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

    void PacketReceiver::RecvMapChunkBulk(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutMapChunkBulk...");

        bool sky_light_sent = PacketDecoder::ReadBoolean(packet);
        int chunk_column_count = PacketDecoder::ReadVarInt(packet);
        std::list<Chunk> chunks = std::list<Chunk>();

        for (int i = 0; i < chunk_column_count; i++)
        {
            int x = PacketDecoder::ReadInt(packet);
            int z = PacketDecoder::ReadInt(packet);
            uint16_t primary_bit_mask = PacketDecoder::ReadShort(packet);
            chunks.push_back(Chunk(x, z, primary_bit_mask));
            this->bot->GetLogger().LogDebug("Loading Chunk (" + std::to_string(x) + "," + std::to_string(z) + ")");
        }

        for (Chunk chunk : chunks)
        {
            PacketDecoder::ReadChunkBulk(chunk, sky_light_sent, packet);
            this->bot->GetChunkRegistry().RegisterValue(std::pair<int, int>(chunk.GetX(), chunk.GetZ()), chunk);
        }
    }

    void PacketReceiver::RecvWorldEvent(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutWorldEvent...");

        int effect_id = PacketDecoder::ReadInt(packet);
        Position location = PacketDecoder::ReadPosition(packet);
        int data = PacketDecoder::ReadInt(packet);
        bool disable_relative_volume = PacketDecoder::ReadBoolean(packet);

        this->bot->GetLogger().LogDebug(
            "Effect ID: " + std::to_string(effect_id) +
            "\n\tPosition: " + location.ToString());
    }

    void PacketReceiver::RecvNamedSoundEffect(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutNamedSoundEffect...");

        std::string sound_name = PacketDecoder::ReadString(packet);
        Vector<int> position = PacketDecoder::ReadVector<int>(packet);
        float volume = PacketDecoder::ReadFloat(packet);
        uint8_t pitch = PacketDecoder::ReadByte(packet);

        this->bot->GetLogger().LogDebug(
            "Sound Name: " + sound_name +
            "\n\tPosition: " + position.ToString());
    }

    void PacketReceiver::RecvWorldParticles(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutWorldParticles...");

        ParticleType particle_id = (ParticleType) PacketDecoder::ReadInt(packet);
        bool long_distance = PacketDecoder::ReadBoolean(packet);
        Vector<float> position = PacketDecoder::ReadVector<float>(packet);
        Vector<float> offset_vec = PacketDecoder::ReadVector<float>(packet);
        float particle_data = PacketDecoder::ReadFloat(packet);
        int particle_count = PacketDecoder::ReadInt(packet);

        int data_length = 0;
        if (particle_id == ParticleType::ICON_CRACK)
        {
            data_length = 2;
        }
        else if (particle_id == ParticleType::BLOCK_CRACK || particle_id == ParticleType::BLOCK_DUST)
        {
            data_length = 1;
        }

        Buffer<int> data = PacketDecoder::ReadVarIntArray(data_length, packet);

        this->bot->GetLogger().LogDebug(
            "Particle ID: " + StringUtils::to_string(particle_id) +
            "\n\tLong Distance: " + std::to_string(long_distance) +
            "\n\tPosition: " + position.ToString() +
            "\n\tOffset: " + offset_vec.ToString() +
            "\n\tParticle Data: " + std::to_string(particle_data)
        );
    }

    void PacketReceiver::RecvGameStateChange(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutGameStateChange...");

        uint8_t reason = PacketDecoder::ReadByte(packet);
        float value = PacketDecoder::ReadFloat(packet);

        this->bot->GetLogger().LogDebug(
            "Reason: " + std::to_string((int)reason) +
            "\n\tValue: " + std::to_string(value));
    }

    void PacketReceiver::RecvSpawnEntityWeather(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutSpawnEntityWeather...");

        int entity_id = PacketDecoder::ReadVarInt(packet);
        WeatherEntityType type = (WeatherEntityType) PacketDecoder::ReadByte(packet);
        Vector<int> position = PacketDecoder::ReadVector<int>(packet);
        Vector<double> position1 = Vector<double>(position.GetX() / 32.0, position.GetY() / 32.0, position.GetZ() / 32.0);

        this->bot->GetLogger().LogDebug(
            "Entity ID: " + std::to_string(entity_id) +
            "\n\tPosition: " + position1.ToString());
    }

    void PacketReceiver::RecvSetSlot(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutSetSlot...");

        uint8_t window_id = PacketDecoder::ReadByte(packet);
        uint16_t slot_number = PacketDecoder::ReadShort(packet);
        Slot slot = PacketDecoder::ReadSlot(packet);

        this->bot->GetLogger().LogDebug(
            "Window ID: " + std::to_string(window_id) +
            "\n\tSlot Number: " + std::to_string(slot_number) +
            "\n\tSlot: " + slot.ToString()
        );
    }

    void PacketReceiver::RecvWindowItems(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutWindowItems...");

        uint8_t window_id = PacketDecoder::ReadByte(packet);
        int16_t count = PacketDecoder::ReadShort(packet);

        if (window_id == 0)
        {
            std::array<Slot, 45> player_inventory;
            for (int i = 0; i < 45; i++)
            {
                player_inventory[i] = PacketDecoder::ReadSlot(packet);
            }

            this->player->SetInventory(player_inventory);
        }
        else
        {
            std::list<Slot> slots = PacketDecoder::ReadSlotArray(count, packet);
        }

        this->bot->GetLogger().LogDebug(
            "Window ID: " + std::to_string(window_id) +
            "\n\tCount: " + std::to_string(count)
        );
    }

    void PacketReceiver::RecvTransaction(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutTransaction...");

        uint8_t window_id = PacketDecoder::ReadByte(packet);
        int16_t action_number = PacketDecoder::ReadShort(packet);
        bool accepted = PacketDecoder::ReadBoolean(packet);

        this->bot->GetLogger().LogDebug(
            "Window ID: " + std::to_string(window_id) +
            "\n\tAction Number: " + std::to_string(action_number) +
            "\n\tAccept: " + std::to_string(accepted)
        );
    }

    void PacketReceiver::RecvUpdateSign(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutUpdateSign...");

        Position location = PacketDecoder::ReadPosition(packet);
        std::string line1 = PacketDecoder::ReadString(packet);
        std::string line2 = PacketDecoder::ReadString(packet);
        std::string line3 = PacketDecoder::ReadString(packet);
        std::string line4 = PacketDecoder::ReadString(packet);

        this->bot->GetLogger().LogDebug(
            "Location: " + location.ToString() +
            "\n\tLine 1: " + line1 +
            "\n\tLine 2: " + line2 +
            "\n\tLine 3: " + line3 +
            "\n\tLine 4: " + line4
        );
    }

    void PacketReceiver::RecvTileEntityData(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutTileEntityData...");

        Position location = PacketDecoder::ReadPosition(packet);
        TileEntityAction action = (TileEntityAction) PacketDecoder::ReadByte(packet);
        NBTTagCompound nbt = PacketDecoder::ReadNBTTagCompound(packet, true);

        this->bot->GetLogger().LogDebug(
            "Location: " + location.ToString() +
            "\n\tAction: " + StringUtils::to_string(action) +
            "\n\tNBT: " + nbt.ToString()
        );
    }

    void PacketReceiver::RecvStatistics(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutStatistics...");
        int count = PacketDecoder::ReadVarInt(packet);
        std::list<Statistic> statistics = PacketDecoder::ReadStatisticArray(count, packet);

        this->bot->GetLogger().LogDebug("Statistics (" + std::to_string(count) + "): ");

        for (auto statistic : statistics)
        {
            this->bot->GetLogger().LogDebug("\t" + statistic.ToString());
        }
    }

    void PacketReceiver::RecvPlayerInfo(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutPlayerInfo...");

        PlayerInfoAction action = (PlayerInfoAction) PacketDecoder::ReadVarInt(packet);
        int players_length = PacketDecoder::ReadVarInt(packet);
        this->bot->UpdatePlayerInfo(action, players_length, packet);
    }

    void PacketReceiver::RecvAbilities(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutAbilities...");
        uint8_t flags = PacketDecoder::ReadByte(packet);
        float flying_speed = PacketDecoder::ReadFloat(packet);
        float fov_modifier = PacketDecoder::ReadFloat(packet);

        this->bot->GetLogger().LogDebug(
            "Flags: " + std::to_string((int)flags) +
            "\n\tFlying Speed: " + std::to_string(flying_speed) +
            "\n\tFOV Modifier: " + std::to_string(fov_modifier));
    }

    void PacketReceiver::RecvScoreboardObjective(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutScoreboardObjective...");
        std::string objective_name = PacketDecoder::ReadString(packet);
        ScoreboardMode mode = (ScoreboardMode) PacketDecoder::ReadByte(packet);

        this->bot->GetLogger().LogDebug(
            "Objective Name: " + objective_name +
            "\n\tMode: " + StringUtils::to_string(mode));

        if (mode == ScoreboardMode::CREATE ||
            mode == ScoreboardMode::UPDATE)
        {
            std::string objective_value = PacketDecoder::ReadString(packet);
            std::string objective_type = PacketDecoder::ReadString(packet);
            this->bot->GetLogger().LogDebug(
                "Objective value: " + objective_value +
                "\n\tObjective type: " + objective_type);
        }
    }

    void PacketReceiver::RecvUpdateScoreboardScore(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutScoreboardScore...");
        std::string score_name = PacketDecoder::ReadString(packet);
        ScoreAction action = (ScoreAction) PacketDecoder::ReadByte(packet);
        std::string objective_name = PacketDecoder::ReadString(packet);

        this->bot->GetLogger().LogDebug(
            "Score Name: " + score_name +
            "\n\tAction: " + StringUtils::to_string(action) +
            "\n\tObjective Name: " + objective_name);

        if (action != ScoreAction::REMOVE)
        {
            int value = PacketDecoder::ReadVarInt(packet);
            this->bot->GetLogger().LogDebug("\n\tValue: " + std::to_string(value));
        }

    }

    void PacketReceiver::RecvDisplayScoreboard(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutScoreboardDisplayObjective...");

        ScoreboardPosition position = (ScoreboardPosition) PacketDecoder::ReadByte(packet);
        std::string score_name = PacketDecoder::ReadString(packet);

        this->bot->GetLogger().LogDebug(
            "Position: " + StringUtils::to_string(position) +
            "\n\tScore Name: " + score_name);
    }

    void PacketReceiver::RecvScoreboardTeam(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutScoreboardTeam...");
        std::string team_name = PacketDecoder::ReadString(packet);
        ScoreboardMode mode = (ScoreboardMode) PacketDecoder::ReadByte(packet);

        this->bot->GetLogger().LogDebug(
            "Team Name: " + team_name +
            "\n\tMode: " + StringUtils::to_string(mode));

        // Creating or updating team
        if (mode == ScoreboardMode::CREATE ||
            mode == ScoreboardMode::UPDATE)
        {
            std::string team_display_name = PacketDecoder::ReadString(packet);
            std::string team_prefix = PacketDecoder::ReadString(packet);
            std::string team_suffix = PacketDecoder::ReadString(packet);

            uint8_t friendly = PacketDecoder::ReadByte(packet);
            this->bot->GetLogger().LogDebug("Friendly: " + std::to_string((int)friendly));
            FriendlyFire friendly_fire = (FriendlyFire) friendly;
            std::string nametag_visibility = PacketDecoder::ReadString(packet);
            uint8_t color = PacketDecoder::ReadByte(packet);

            this->bot->GetLogger().LogDebug(
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
            int player_count = PacketDecoder::ReadVarInt(packet);
            std::list<std::string> players = PacketDecoder::ReadStringArray(player_count, packet);

            this->bot->GetLogger().LogDebug("Players (" + std::to_string(player_count) + "): ");

            for (auto player : players)
            {
                this->bot->GetLogger().LogDebug("\t\t" + player);
            }
        }

    }

    void PacketReceiver::RecvPluginMessage(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutCustomPayload...");

        std::string plugin_channel = PacketDecoder::ReadString(packet);
        Buffer<uint8_t> data = PacketDecoder::ReadByteArray(packet.length - packet.offset, packet);

        this->bot->GetLogger().LogDebug(
            "Plugin Channel: " + plugin_channel +
            "\n\tData: " + data.to_string());
    }

    void PacketReceiver::RecvServerDifficulty(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutServerDifficulty...");

        Difficulty difficulty = (Difficulty) PacketDecoder::ReadByte(packet);

        this->bot->GetLogger().LogDebug("Difficulty: " + StringUtils::to_string(difficulty));
    }

    void PacketReceiver::RecvWorldBorder(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutWorldBorder...");

        WorldBorderAction action = (WorldBorderAction) PacketDecoder::ReadVarInt(packet);

        switch (action)
        {
        case WorldBorderAction::SET_SIZE:
        {
            double radius = PacketDecoder::ReadDouble(packet);
            //this->world_border.set_radius(radius);
            break;
        }

        case WorldBorderAction::LERP_SIZE:
        {
            double old_radius = PacketDecoder::ReadDouble(packet);
            double new_radius = PacketDecoder::ReadDouble(packet);
            long speed = PacketDecoder::ReadVarLong(packet);
            //this->world_border.set_old_radius(old_radius);
            //this->world_border.set_new_radius(new_radius);
            //this->world_border.set_speed(speed);
            break;
        }

        case WorldBorderAction::SET_CENTER:
        {
            double x = PacketDecoder::ReadDouble(packet);
            double z = PacketDecoder::ReadDouble(packet);
            //this->world_border.set_x(x);
            //this->world_border.set_z(z);
            break;
        }

        case WorldBorderAction::INITIALIZE:
        {
            double x = PacketDecoder::ReadDouble(packet);
            double z = PacketDecoder::ReadDouble(packet);
            double old_radius = PacketDecoder::ReadDouble(packet);
            double new_radius = PacketDecoder::ReadDouble(packet);
            long speed = PacketDecoder::ReadVarLong(packet);
            int portal_teleport_boundary = PacketDecoder::ReadVarInt(packet);
            int warning_time = PacketDecoder::ReadVarInt(packet);
            int warning_blocks = PacketDecoder::ReadVarInt(packet);

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
            int warning_time = PacketDecoder::ReadVarInt(packet);
            //this->world_border.set_warning_time(warning_time);
            break;
        }

        case WorldBorderAction::SET_WARNING_BLOCKS:
        {
            int warning_blocks = PacketDecoder::ReadVarInt(packet);
            //this->world_border.set_warning_blocks(warning_blocks);
            break;
        }

        }

    }

    void PacketReceiver::RecvTitle(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutTitle...");

        TitleAction action = (TitleAction) PacketDecoder::ReadVarInt(packet);

        switch (action)
        {
        case TitleAction::SET_TITLE:
        {
            std::string title_text = PacketDecoder::ReadString(packet);
            break;
        }
        case TitleAction::SET_SUBTITLE:
        {
            std::string subtitle_text = PacketDecoder::ReadString(packet);
            break;
        }
        case TitleAction::SET_TIMES_AND_DISPLAY:
        {
            int fade_in = PacketDecoder::ReadInt(packet);
            int stay = PacketDecoder::ReadInt(packet);
            int fade_out = PacketDecoder::ReadInt(packet);
            break;
        }

        }

        this->bot->GetLogger().LogDebug("Action: " + StringUtils::to_string(action));
    }

    void PacketReceiver::RecvPlayerListHeaderFooter(Packet packet)
    {
        this->bot->GetLogger().LogDebug("<<< Handling PacketPlayOutPlayerListHeaderFooter...");

        std::string header = PacketDecoder::ReadString(packet);
        std::string footer = PacketDecoder::ReadString(packet);

        this->bot->GetLogger().LogDebug(
            "Header: " + header +
            "\n\tFooter: " + footer);
    }
}
