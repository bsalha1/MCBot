#include "MCBot.h"
#include "PacketReceiver.h"
#include "PacketSender.h"
#include "StringUtils.h"

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

mcbot::PacketReceiver::PacketReceiver(MCBot* bot) : player(&(bot->get_player()))
{
    this->bot = bot;
}

mcbot::PacketReceiver::PacketReceiver() : player(NULL)
{
    this->bot = NULL;
}


int mcbot::PacketReceiver::read_next_var_int()
{
    int num_read = 0;
    int result = 0;
    char read;
    int i = 0;
    do {

        uint8_t packet[2] = { 0 };
        size_t bytes_read = this->bot->get_socket().recv_packet(packet, 1);
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

void mcbot::PacketReceiver::recv_packet()
{
    int length = this->read_next_var_int();
    this->bot->log_debug("Length: " + std::to_string(length));

    uint8_t* packet;
    int decompressed_length = 0;
    if (this->compression_enabled)
    {
        decompressed_length = this->read_next_var_int();
        length -= PacketEncoder::get_var_int_size(decompressed_length);
        bot->log_debug("Decompressed length: " + std::to_string(decompressed_length));
    }

    int packet_size;
    if (decompressed_length == 0)
    {
        packet = new uint8_t[length]{ 0 };
        packet_size = this->read_next_packet(length, packet);
    }
    else
    {
        packet = new uint8_t[decompressed_length]{ 0 };
        packet_size = this->read_next_packet(length, packet, decompressed_length);
    }

    size_t offset = 0;
    int packet_id = PacketDecoder::read_var_int(packet, offset);
    bot->log_debug("ID: " + std::to_string(packet_id));

    handle_recv_packet(packet_id, packet, packet_size, offset);

    delete[] packet;
}

int mcbot::PacketReceiver::read_next_packet(int length, uint8_t* packet, int decompressed_length)
{
    int bytes_read = this->bot->get_socket().recv_packet(packet, length, decompressed_length);
    if (bytes_read < 0)
    {
        bot->log_error("Failed to receive packet");
        packet = NULL;
        return bytes_read;
    }
    bot->log_debug("Received Packet: " + std::to_string(bytes_read) + "bytes");

    return bytes_read;
}

void mcbot::PacketReceiver::handle_recv_packet(int packet_id, uint8_t* packet, int length, size_t& offset)
{
    if (this->bot->get_state() == State::HANDSHAKE)
    {

    }
    else if (this->bot->get_state() == State::STATUS)
    {

    }
    else if (this->bot->get_state() == State::LOGIN)
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
            bot->log_error("Unhandled " + StringUtils::to_string(this->bot->get_state()) + " packet id: " + std::to_string(packet_id));
        }
    }
    else if (this->bot->get_state() == State::PLAY)
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
            this->recv_block_break_animation(packet, length, offset);
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
        case 0x2C:
            this->recv_spawn_entity_weather(packet, length, offset);
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
            bot->log_error("Unhandled " + StringUtils::to_string(this->bot->get_state()) + " packet id: " + std::to_string(packet_id));
        }
    }
    else
    {
        bot->log_error("Unknown state: " + StringUtils::to_string(this->bot->get_state()));
    }
}

void mcbot::PacketReceiver::recv_login_disconnect(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketLoginOutDisconnect...");

    std::cout << packet << std::endl;

    this->bot->set_connected(false);
    bot->log_info("Disconnected");
}

void mcbot::PacketReceiver::recv_set_compression(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketLoginOutSetCompression...");
    int max_uncompressed_length = PacketDecoder::read_var_int(packet, offset);

    bot->log_debug("\tMax Uncompressed Length: " + std::to_string(max_uncompressed_length));
    this->compression_enabled = true;
    this->bot->get_socket().initialize_compression(max_uncompressed_length);
    bot->log_debug("<<< COMPRESSION ENABLED <<<");
}

void mcbot::PacketReceiver::recv_encryption_request(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketLoginOutEncryptionRequest...");

    // Server ID //
    std::string server_id = PacketDecoder::read_string(packet, offset);

    // Public Key //
    int public_key_length = PacketDecoder::read_var_int(packet, offset);
    uint8_t* public_key = new uint8_t[public_key_length];
    PacketDecoder::read_byte_array(public_key, public_key_length, packet, offset);

    // Verify Token //
    int verify_token_length = PacketDecoder::read_var_int(packet, offset);
    uint8_t* verify_token = new uint8_t[verify_token_length];
    PacketDecoder::read_byte_array(verify_token, verify_token_length, packet, offset);

    std::string shared_secret = get_random_hex_bytes(16);

    // Save Session //
    // - So Yggdrasil authentication doesn't kick us!
    bot->log_debug("Saving session...");
    if (this->bot->get_packet_sender().send_session(server_id, shared_secret, public_key, public_key_length) < 0)
    {
        bot->log_error("Invalid session!");
    }
    else
    {
        this->bot->get_packet_sender().send_encryption_response(public_key, public_key_length, verify_token, verify_token_length, shared_secret);
    }
}

void mcbot::PacketReceiver::recv_login_success(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayLoginOutSuccess...");

    std::string uuid_string = PacketDecoder::read_string(packet, offset);
    std::string username = PacketDecoder::read_string(packet, offset);

    bot->log_debug(
        "UUID: " + uuid_string + '\n' +
        "\tUsername: " + username);

    bot->log_info("Successfully logged in!");

    this->bot->set_state(State::PLAY);
}

void mcbot::PacketReceiver::recv_play_disconnect(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutKickDisconnect...");

    std::string reason = PacketDecoder::read_string(packet, offset);

    bot->log_info("Disconnected: " + reason);

    this->bot->set_connected(false);
}

void mcbot::PacketReceiver::recv_keep_alive(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutKeepAlive...");

    int id = PacketDecoder::read_var_int(packet, offset);

    this->bot->get_packet_sender().send_keep_alive(id);
}

void mcbot::PacketReceiver::recv_join_server(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutJoinServer...");

    int entity_id = PacketDecoder::read_int(packet, offset);
    mcbot::Gamemode gamemode = (mcbot::Gamemode) PacketDecoder::read_byte(packet, offset);
    mcbot::Dimension dimension = (mcbot::Dimension) PacketDecoder::read_byte(packet, offset);
    mcbot::Difficulty difficulty = (mcbot::Difficulty) PacketDecoder::read_byte(packet, offset);
    uint8_t max_players = PacketDecoder::read_byte(packet, offset);
    std::string level_type = PacketDecoder::read_string(packet, offset);
    bool reduced_debug_info = PacketDecoder::read_boolean(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tGamemode: " + StringUtils::to_string(gamemode) +
        "\n\tDimension: " + StringUtils::to_string(dimension) +
        "\n\tDifficulty: " + StringUtils::to_string(difficulty) +
        "\n\tMax Players: " + std::to_string((int)max_players) +
        "\n\tLevel Type: " + level_type +
        "\n\tReduced Debug Info: " + std::to_string(reduced_debug_info));


}

void mcbot::PacketReceiver::recv_chat_message(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutChat...");

    std::string chat_data = PacketDecoder::read_string(packet, offset);
    mcbot::ChatPosition position = (mcbot::ChatPosition) PacketDecoder::read_byte(packet, offset);

    bot->log_debug(
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

    bot->log_chat(chat_message);
}

void mcbot::PacketReceiver::recv_update_time(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutUpdateTime...");

    int64_t world_age = (int64_t)PacketDecoder::read_long(packet, offset);
    int64_t time_of_day = (int64_t)PacketDecoder::read_long(packet, offset);

    bot->log_debug(
        "World Age: " + std::to_string(world_age) +
        "\n\tTime of Day: " + std::to_string(time_of_day));
}

void mcbot::PacketReceiver::recv_entity_equipment(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutEntityEquipment...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    short slot = PacketDecoder::read_short(packet, offset);
    mcbot::Slot item = PacketDecoder::read_slot(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tSlot: " + std::to_string(slot) +
        "\n\tItem: " + item.to_string());
}

void mcbot::PacketReceiver::recv_spawn_position(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutSpawnPosition...");

    mcbot::Position location = PacketDecoder::read_position(packet, offset);

    bot->log_debug("Location: " + location.to_string());
}

void mcbot::PacketReceiver::recv_update_health(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutUpdateHealth...");

    float health = PacketDecoder::read_float(packet, offset);
    int food = PacketDecoder::read_var_int(packet, offset);
    float food_saturation = PacketDecoder::read_float(packet, offset);

    bot->log_debug("Health: " + std::to_string(health) +
        "\n\tFood: " + std::to_string(food) +
        "\n\tSaturation: " + std::to_string(food_saturation));
}


void mcbot::PacketReceiver::recv_position(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutPosition...");

    mcbot::Vector<double> position = PacketDecoder::read_vector<double>(packet, offset);
    float yaw = PacketDecoder::read_float(packet, offset);
    float pitch = PacketDecoder::read_float(packet, offset);
    uint8_t flags = PacketDecoder::read_byte(packet, offset);

    bot->log_debug("Position: " + position.to_string() +
        "\n\tYaw: " + std::to_string(yaw) +
        "\n\tPitch: " + std::to_string(pitch) +
        "\n\tFlags: " + std::to_string((int)flags));

    this->bot->get_player().update_location(position);
    this->bot->get_player().update_rotation(yaw, pitch);
}

void mcbot::PacketReceiver::recv_held_item_slot(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutHeldItemSlot...");

    uint8_t held_item_slot = PacketDecoder::read_byte(packet, offset);

    bot->log_debug("Held Item Slot: " + std::to_string((int)held_item_slot));
}

void mcbot::PacketReceiver::recv_bed(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutBed...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Position position = PacketDecoder::read_position(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tLocation: " + position.to_string());
}

void mcbot::PacketReceiver::recv_animation(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutAnimation...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Position position = PacketDecoder::read_position(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tLocation: " + position.to_string());
}

void mcbot::PacketReceiver::recv_named_entity_spawn(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutNamedEntitySpawn...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    UUID uuid = PacketDecoder::read_uuid(packet, offset);
    int x = PacketDecoder::read_int(packet, offset);
    int y = PacketDecoder::read_int(packet, offset);
    int z = PacketDecoder::read_int(packet, offset);
    mcbot::Vector<double> position = mcbot::Vector<double>(x / 32.0, y / 32.0, z / 32.0);

    uint8_t pitch = PacketDecoder::read_byte(packet, offset);
    uint8_t yaw = PacketDecoder::read_byte(packet, offset);
    short current_item = PacketDecoder::read_short(packet, offset);
    EntityMetaData meta_data = PacketDecoder::read_meta_data(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tUUID: " + uuid.to_string() +
        "\n\tLocation: " + position.to_string());

    if (!this->bot->is_player_registered(uuid))
    {
        EntityPlayer player = EntityPlayer(entity_id, uuid);
        player.update_location(position);
        player.update_rotation(yaw, pitch);
        this->bot->register_player(uuid, player);
    }
    else
    {
        EntityPlayer& player = this->bot->get_player(uuid);
        player.set_id(entity_id);
        player.update_location(position);
        player.update_rotation(yaw, pitch);
    }
}

void mcbot::PacketReceiver::recv_collect(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutCollect...");

    int collected_id = PacketDecoder::read_var_int(packet, offset);
    int collector_id = PacketDecoder::read_var_int(packet, offset);

    bot->log_debug(
        "Collected ID: " + std::to_string(collected_id) +
        "\n\tCollector ID: " + std::to_string(collector_id));
}

void mcbot::PacketReceiver::recv_spawn_entity(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutSpawnEntity...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    EntityType type = (EntityType)PacketDecoder::read_byte(packet, offset);
    mcbot::Vector<int> position = PacketDecoder::read_vector<int>(packet, offset);
    mcbot::Vector<double> position1 = mcbot::Vector<double>(position.get_x() / 32.0, position.get_y() / 32.0, position.get_z() / 32.0);

    uint8_t pitch = PacketDecoder::read_byte(packet, offset);
    uint8_t yaw = PacketDecoder::read_byte(packet, offset);
    int data = PacketDecoder::read_int(packet, offset);

    if (data > 0)
    {
        mcbot::Vector<short> motion = PacketDecoder::read_vector<short>(packet, offset);
    }

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tLocation:" + position.to_string());

    if (this->bot->is_entity_registered(entity_id))
    {
        this->bot->get_entity(entity_id).update_location(position1);
    }
    else
    {
        Entity entity = Entity(type, entity_id);
        entity.update_location(position1);
        this->bot->register_entity(entity);
    }
}

void mcbot::PacketReceiver::recv_spawn_entity_living(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutSpawnEntityLiving...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    EntityType type = (EntityType)PacketDecoder::read_byte(packet, offset);
    mcbot::Vector<int> position = PacketDecoder::read_vector<int>(packet, offset);
    mcbot::Vector<double> position1 = mcbot::Vector<double>(position.get_x() / 32.0, position.get_y() / 32.0, position.get_z() / 32.0);

    uint8_t pitch = PacketDecoder::read_byte(packet, offset);
    uint8_t yaw = PacketDecoder::read_byte(packet, offset);
    uint8_t head_pitch = PacketDecoder::read_byte(packet, offset);
    mcbot::Vector<short> motion = PacketDecoder::read_vector<short>(packet, offset);
    EntityMetaData meta_data = PacketDecoder::read_meta_data(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tLocation:" + position.to_string());

    if (this->bot->is_entity_registered(entity_id))
    {
        this->bot->get_entity(entity_id).update_location(position1);
    }
    else
    {
        EntityLiving entity = EntityLiving(type, entity_id);
        entity.update_location(position1);
        this->bot->register_entity(entity);
    }
}

void mcbot::PacketReceiver::recv_spawn_entity_painting(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutSpawnEntityPainting...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    std::string title = PacketDecoder::read_string(packet, offset);
    mcbot::Position location = PacketDecoder::read_position(packet, offset);
    uint8_t direction = PacketDecoder::read_byte(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tLocation: " + location.to_string() +
        "\n\tTitle: " + title +
        "\n\tDirection: " + std::to_string((int)direction));

    this->bot->register_entity(Entity(EntityType::PAINTING, entity_id));
}

void mcbot::PacketReceiver::recv_spawn_entity_experience_orb(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutSpawnEntityExperienceOrb...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Vector<int> motion = PacketDecoder::read_vector<int>(packet, offset);
    short count = PacketDecoder::read_short(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tPosition: " + motion.to_string() +
        "\n\tCount: " + std::to_string(count));

    this->bot->register_entity(Entity(EntityType::EXPERIENCE_ORB, entity_id));
}

void mcbot::PacketReceiver::recv_entity_velocity(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutEntityVelocity...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Vector<short> motion = PacketDecoder::read_vector<short>(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tVelocity: " + motion.to_string());
}

void mcbot::PacketReceiver::recv_entity_destroy(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutEntityDestroy...");

    int count = PacketDecoder::read_var_int(packet, offset);

    std::list<int> entity_ids = std::list <int>();
    for (int i = 0; i < count; i++)
    {
        entity_ids.push_back(PacketDecoder::read_var_int(packet, offset));
    }

    bot->log_debug(
        "Entity Count: " + std::to_string(count));

    for (int id : entity_ids)
    {
        mcbot::Entity& entity = this->bot->get_entity(id);
        entity.die();
        this->bot->remove_entity(id);

        if (entity.get_entity_type() == mcbot::EntityType::PLAYER)
        {
            this->bot->unregister_player(static_cast<EntityPlayer&>(entity));
        }
    }
}

void mcbot::PacketReceiver::recv_entity(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutEntity...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id));

    this->bot->register_entity(Entity(entity_id));
}

void mcbot::PacketReceiver::recv_rel_entity_move(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutRelEntityMove...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Vector<int8_t> dmot = PacketDecoder::read_vector<int8_t>(packet, offset);
    bool on_ground = PacketDecoder::read_boolean(packet, offset);

    mcbot::Vector<double> dr = mcbot::Vector<double>(
        dmot.get_x() / 32.0, dmot.get_y() / 32.0, dmot.get_z() / 32.0);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tChange in Motion: " + dr.to_string() +
        "\n\tOn Ground: " + std::to_string(on_ground));

    this->bot->get_entity(entity_id).update_motion(dr);
}

void mcbot::PacketReceiver::recv_entity_look(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutEntityLook...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    uint8_t yaw = PacketDecoder::read_byte(packet, offset);
    uint8_t pitch = PacketDecoder::read_byte(packet, offset);
    bool on_ground = PacketDecoder::read_boolean(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tYaw: " + std::to_string((int)yaw) +
        "\n\tPitch: " + std::to_string((int)pitch) +
        "\n\tOn Ground: " + std::to_string(on_ground));

    this->bot->get_entity(entity_id).update_rotation(yaw, pitch);
}

void mcbot::PacketReceiver::recv_rel_entity_move_look(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutRelEntityMoveLook...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Vector<int8_t> dmot = PacketDecoder::read_vector<int8_t>(packet, offset);
    mcbot::Vector<double> dr = mcbot::Vector<double>(dmot.get_x() / 32.0, dmot.get_y() / 32.0, dmot.get_z() / 32.0);
    double yaw = PacketDecoder::read_byte(packet, offset) * 2 * 3.14159 / 256;
    uint8_t pitch = PacketDecoder::read_byte(packet, offset) * 2 * 3.14159 / 256;
    bool on_ground = PacketDecoder::read_boolean(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tChange in Motion: " + dr.to_string() +
        "\n\tYaw: " + std::to_string(yaw) +
        "\n\tPitch: " + std::to_string(pitch) +
        "\n\tOn Ground: " + std::to_string(on_ground));

    this->bot->get_entity(entity_id).update_motion(dr);
    this->bot->get_entity(entity_id).update_rotation(yaw, pitch);
}

void mcbot::PacketReceiver::recv_entity_teleport(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutEntityTeleport...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Vector<int> position = PacketDecoder::read_vector<int>(packet, offset);
    mcbot::Vector<double> position1 = mcbot::Vector<double>(position.get_x() / 32.0, position.get_y() / 32.0, position.get_z() / 32.0);

    double yaw = PacketDecoder::read_byte(packet, offset) * 2 * 3.14159 / 256;
    double pitch = PacketDecoder::read_byte(packet, offset) * 2 * 3.14159 / 256;
    bool on_ground = PacketDecoder::read_boolean(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tChange in Motion: " + position.to_string() +
        "\n\tYaw: " + std::to_string(yaw) +
        "\n\tPitch: " + std::to_string(pitch) +
        "\n\tOn Ground: " + std::to_string(on_ground));

    this->bot->get_entity(entity_id).update_location(position1);
    this->bot->get_entity(entity_id).update_rotation(yaw, pitch);
}

void mcbot::PacketReceiver::recv_entity_head_look(uint8_t* packet, size_t length, size_t& offset)
{
    // TODO: find actual name of packet
    bot->log_debug("<<< Handling PacketPlayOutEntityHeadLook...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    double yaw = PacketDecoder::read_byte(packet, offset) * 2 * 3.14159 / 256;

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tAngle: " + std::to_string(yaw));

    this->bot->get_entity(entity_id).update_yaw(yaw);
}

void mcbot::PacketReceiver::recv_entity_status(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutEntityStatus...");

    int entity_id = PacketDecoder::read_int(packet, offset);
    mcbot::EntityStatus status = (mcbot::EntityStatus) PacketDecoder::read_byte(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tStatus: " + StringUtils::to_string(status));
}

void mcbot::PacketReceiver::recv_entity_metadata(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutEntityMetadata...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::EntityMetaData meta_data = PacketDecoder::read_meta_data(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tMeta Data: size " + std::to_string(meta_data.get_values().size()));
}

void mcbot::PacketReceiver::recv_entity_effect(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutEntityEffect...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    uint8_t effect_id = PacketDecoder::read_byte(packet, offset);
    uint8_t amplifier = PacketDecoder::read_byte(packet, offset);
    int duration = PacketDecoder::read_var_int(packet, offset);
    bool hide_particles = PacketDecoder::read_boolean(packet, offset);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tEffect ID: " + std::to_string((int)effect_id) +
        "\n\tAmplifier: " + std::to_string((int)amplifier) +
        "\n\tDuration: " + std::to_string(duration) +
        "\n\tHide Particles: " + std::to_string(hide_particles));
}

void mcbot::PacketReceiver::recv_experience(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutExperience...");

    float experience_bar = PacketDecoder::read_float(packet, offset);
    int level = PacketDecoder::read_var_int(packet, offset);
    int total_experience = PacketDecoder::read_var_int(packet, offset);

    bot->log_debug(
        "Experience Bar: " + std::to_string(experience_bar) +
        "\n\tLevel: " + std::to_string(level) +
        "\n\tTotal Experience: " + std::to_string(total_experience));
}

void mcbot::PacketReceiver::recv_entity_attributes(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutUpdateAttributes...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    int num_attributes = PacketDecoder::read_int(packet, offset);

    std::list<mcbot::Attribute> attributes;
    for (int i = 0; i < num_attributes; i++)
    {
        attributes.push_back(PacketDecoder::read_attribute(packet, offset));
    }

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tAttributes: " + std::to_string(attributes.size()));

}

void mcbot::PacketReceiver::recv_map_chunk(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutMapChunk...");

    int chunk_x = PacketDecoder::read_int(packet, offset);
    int chunk_z = PacketDecoder::read_int(packet, offset);
    bool ground_up_continuous = PacketDecoder::read_boolean(packet, offset);
    uint16_t primary_bitmask = PacketDecoder::read_short(packet, offset);
    int data_size = PacketDecoder::read_var_int(packet, offset);
    Chunk chunk = PacketDecoder::read_chunk(chunk_x, chunk_z, ground_up_continuous, true, primary_bitmask, packet, offset);
    
    bot->log_debug(
        "X: " + std::to_string(chunk_x) +
        "\n\tZ: " + std::to_string(chunk_z));

    this->bot->load_chunk(chunk);
}

void mcbot::PacketReceiver::recv_multi_block_change(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutMultiBlockChange...");

    int chunk_x = PacketDecoder::read_int(packet, offset);
    int chunk_z = PacketDecoder::read_int(packet, offset);
    int record_count = PacketDecoder::read_var_int(packet, offset);

    mcbot::Chunk& chunk = this->bot->get_chunk(chunk_x, chunk_z);
    for (int i = 0; i < record_count; i++)
    {
        uint8_t horizontal_position = PacketDecoder::read_byte(packet, offset);
        uint8_t y = PacketDecoder::read_byte(packet, offset);
        int block_id = PacketDecoder::read_var_int(packet, offset);

        int x = horizontal_position >> 4 + chunk_x << 4;
        int z = horizontal_position & 0x0F + chunk_z << 4;

        chunk.update_block(x, y, z, block_id);
    }

    bot->log_debug(
        "X: " + std::to_string(chunk_x) +
        "\n\tZ: " + std::to_string(chunk_z));
}

void mcbot::PacketReceiver::recv_block_change(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutBlockChange...");

    mcbot::Position location = PacketDecoder::read_position(packet, offset);
    int block_id = PacketDecoder::read_var_int(packet, offset);

    bot->log_debug(
        "Location: " + location.to_string() +
        "\n\tBlock ID: " + std::to_string(block_id));

    Chunk& chunk = this->bot->get_chunk(location.to_vector());
    chunk.update_block(location.to_vector(), block_id);
}

void mcbot::PacketReceiver::recv_block_break_animation(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutBlockBreakAnimation...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::Position location = PacketDecoder::read_position(packet, offset);
    uint8_t destroy_stage = PacketDecoder::read_byte(packet, offset);

    bot->log_debug(
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

void mcbot::PacketReceiver::recv_map_chunk_bulk(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutMapChunkBulk...");

    bool sky_light_sent = PacketDecoder::read_boolean(packet, offset);
    int chunk_column_count = PacketDecoder::read_var_int(packet, offset);
    std::list<Chunk> chunks = std::list<Chunk>();

    for (int i = 0; i < chunk_column_count; i++)
    {
        int x = PacketDecoder::read_int(packet, offset);
        int z = PacketDecoder::read_int(packet, offset);
        uint16_t primary_bit_mask = PacketDecoder::read_short(packet, offset);
        chunks.push_back(Chunk(x, z, primary_bit_mask));
        bot->log_debug("Loading Chunk (" + std::to_string(x) + "," + std::to_string(z) + ")");
    }

    for (Chunk chunk : chunks)
    {
        PacketDecoder::read_chunk_bulk(chunk, sky_light_sent, packet, offset);
        this->bot->load_chunk(chunk);
    }
}

void mcbot::PacketReceiver::recv_world_event(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutWorldEvent...");

    int effect_id = PacketDecoder::read_int(packet, offset);
    mcbot::Position location = PacketDecoder::read_position(packet, offset);
    int data = PacketDecoder::read_int(packet, offset);
    bool disable_relative_volume = PacketDecoder::read_boolean(packet, offset);

    bot->log_debug(
        "Effect ID: " + std::to_string(effect_id) +
        "\n\tPosition: " + location.to_string());
}

void mcbot::PacketReceiver::recv_named_sound_effect(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutNamedSoundEffect...");

    std::string sound_name = PacketDecoder::read_string(packet, offset);
    mcbot::Vector<int> position = PacketDecoder::read_vector<int>(packet, offset);
    float volume = PacketDecoder::read_float(packet, offset);
    uint8_t pitch = PacketDecoder::read_byte(packet, offset);

    bot->log_debug(
        "Sound Name: " + sound_name +
        "\n\tPosition: " + position.to_string());
}

void mcbot::PacketReceiver::recv_world_particles(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutWorldParticles...");

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

    bot->log_debug(
        "Particle ID: " + StringUtils::to_string(particle_id) +
        "\n\tLong Distance: " + std::to_string(long_distance) +
        "\n\tPosition: " + position.to_string() +
        "\n\tOffset: " + offset_vec.to_string() +
        "\n\tParticle Data: " + std::to_string(particle_data)
    );
}

void mcbot::PacketReceiver::recv_game_state_change(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutGameStateChange...");

    uint8_t reason = PacketDecoder::read_byte(packet, offset);
    float value = PacketDecoder::read_float(packet, offset);

    bot->log_debug(
        "Reason: " + std::to_string((int)reason) +
        "\n\tValue: " + std::to_string(value));
}

void mcbot::PacketReceiver::recv_spawn_entity_weather(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutSpawnEntityWeather...");

    int entity_id = PacketDecoder::read_var_int(packet, offset);
    mcbot::WeatherEntityType type = (mcbot::WeatherEntityType) PacketDecoder::read_byte(packet, offset);
    mcbot::Vector<int> position = PacketDecoder::read_vector<int>(packet, offset);
    mcbot::Vector<double> position1 = mcbot::Vector<double>(position.get_x() / 32.0, position.get_y() / 32.0, position.get_z() / 32.0);

    bot->log_debug(
        "Entity ID: " + std::to_string(entity_id) +
        "\n\tPosition: " + position1.to_string());
}

void mcbot::PacketReceiver::recv_set_slot(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutSetSlot...");

    uint8_t window_id = PacketDecoder::read_byte(packet, offset);
    uint16_t slot_number = PacketDecoder::read_short(packet, offset);
    mcbot::Slot slot = PacketDecoder::read_slot(packet, offset);

    bot->log_debug(
        "Window ID: " + std::to_string(window_id) +
        "\n\tSlot Number: " + std::to_string(slot_number) +
        "\n\tSlot: " + slot.to_string()
    );
}

void mcbot::PacketReceiver::recv_window_items(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutWindowItems...");

    uint8_t window_id = PacketDecoder::read_byte(packet, offset);
    int16_t count = PacketDecoder::read_short(packet, offset);
    std::list<mcbot::Slot> slots = PacketDecoder::read_slot_array(count, packet, offset);

    bot->log_debug(
        "Window ID: " + std::to_string(window_id) +
        "\n\tCount: " + std::to_string(count)
    );
}

void mcbot::PacketReceiver::recv_transaction(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutTransaction...");

    uint8_t window_id = PacketDecoder::read_byte(packet, offset);
    int16_t action_number = PacketDecoder::read_short(packet, offset);
    bool accepted = PacketDecoder::read_boolean(packet, offset);

    bot->log_debug(
        "Window ID: " + std::to_string(window_id) +
        "\n\tAction Number: " + std::to_string(action_number) +
        "\n\tAccept: " + std::to_string(accepted)
    );
}

void mcbot::PacketReceiver::recv_update_sign(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutUpdateSign...");

    mcbot::Position location = PacketDecoder::read_position(packet, offset);
    std::string line1 = PacketDecoder::read_string(packet, offset);
    std::string line2 = PacketDecoder::read_string(packet, offset);
    std::string line3 = PacketDecoder::read_string(packet, offset);
    std::string line4 = PacketDecoder::read_string(packet, offset);

    bot->log_debug(
        "Location: " + location.to_string() +
        "\n\tLine 1: " + line1 +
        "\n\tLine 2: " + line2 +
        "\n\tLine 3: " + line3 +
        "\n\tLine 4: " + line4
    );
}

void mcbot::PacketReceiver::recv_tile_entity_data(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutTileEntityData...");

    mcbot::Position location = PacketDecoder::read_position(packet, offset);
    mcbot::TileEntityAction action = (mcbot::TileEntityAction) PacketDecoder::read_byte(packet, offset);
    mcbot::NBTTagCompound nbt = PacketDecoder::read_nbt_tag_compound(packet, offset);

    bot->log_debug(
        "Location: " + location.to_string() +
        "\n\tAction: " + StringUtils::to_string(action) +
        "\n\tNBT: " + nbt.to_string()
    );
}

void mcbot::PacketReceiver::recv_statistics(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutStatistics...");
    int count = PacketDecoder::read_var_int(packet, offset);
    std::list<mcbot::Statistic> statistics = PacketDecoder::read_statistic_array(count, packet, offset);

    bot->log_debug("Statistics (" + std::to_string(count) + "): ");

    for (auto statistic : statistics)
    {
        bot->log_debug("\t" + statistic.to_string());
    }
}

void mcbot::PacketReceiver::recv_player_info(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutPlayerInfo...");

    mcbot::PlayerInfoAction action = (mcbot::PlayerInfoAction) PacketDecoder::read_var_int(packet, offset);
    int players_length = PacketDecoder::read_var_int(packet, offset);
    this->bot->update_player_info(action, players_length, packet, offset);
}

void mcbot::PacketReceiver::recv_abilities(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutAbilities...");
    uint8_t flags = PacketDecoder::read_byte(packet, offset);
    float flying_speed = PacketDecoder::read_float(packet, offset);
    float fov_modifier = PacketDecoder::read_float(packet, offset);

    bot->log_debug(
        "Flags: " + std::to_string((int)flags) +
        "\n\tFlying Speed: " + std::to_string(flying_speed) +
        "\n\tFOV Modifier: " + std::to_string(fov_modifier));
}

void mcbot::PacketReceiver::recv_scoreboard_objective(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutScoreboardObjective...");
    std::string objective_name = PacketDecoder::read_string(packet, offset);
    mcbot::ScoreboardMode mode = (mcbot::ScoreboardMode) PacketDecoder::read_byte(packet, offset);

    bot->log_debug(
        "Objective Name: " + objective_name +
        "\n\tMode: " + StringUtils::to_string(mode));

    if (mode == mcbot::ScoreboardMode::CREATE ||
        mode == mcbot::ScoreboardMode::UPDATE)
    {
        std::string objective_value = PacketDecoder::read_string(packet, offset);
        std::string objective_type = PacketDecoder::read_string(packet, offset);
        bot->log_debug(
            "Objective value: " + objective_value +
            "\n\tObjective type: " + objective_type);
    }
}

void mcbot::PacketReceiver::recv_update_scoreboard_score(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutScoreboardScore...");
    std::string score_name = PacketDecoder::read_string(packet, offset);
    mcbot::ScoreAction action = (mcbot::ScoreAction) PacketDecoder::read_byte(packet, offset);
    std::string objective_name = PacketDecoder::read_string(packet, offset);

    bot->log_debug(
        "Score Name: " + score_name +
        "\n\tAction: " + StringUtils::to_string(action) +
        "\n\tObjective Name: " + objective_name);

    if (action != mcbot::ScoreAction::REMOVE)
    {
        int value = PacketDecoder::read_var_int(packet, offset);
        bot->log_debug("\n\tValue: " + std::to_string(value));
    }

}

void mcbot::PacketReceiver::recv_display_scoreboard(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutScoreboardDisplayObjective...");

    mcbot::ScoreboardPosition position = (mcbot::ScoreboardPosition) PacketDecoder::read_byte(packet, offset);
    std::string score_name = PacketDecoder::read_string(packet, offset);

    bot->log_debug(
        "Position: " + StringUtils::to_string(position) +
        "\n\tScore Name: " + score_name);
}

void mcbot::PacketReceiver::recv_scoreboard_team(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutScoreboardTeam...");
    std::string team_name = PacketDecoder::read_string(packet, offset);
    mcbot::ScoreboardMode mode = (mcbot::ScoreboardMode) PacketDecoder::read_byte(packet, offset);

    bot->log_debug(
        "Team Name: " + team_name +
        "\n\tMode: " + StringUtils::to_string(mode));

    // Creating or updating team
    if (mode == mcbot::ScoreboardMode::CREATE ||
        mode == mcbot::ScoreboardMode::UPDATE)
    {
        std::string team_display_name = PacketDecoder::read_string(packet, offset);
        std::string team_prefix = PacketDecoder::read_string(packet, offset);
        std::string team_suffix = PacketDecoder::read_string(packet, offset);

        uint8_t friendly = PacketDecoder::read_byte(packet, offset);
        bot->log_debug("Friendly: " + std::to_string((int)friendly));
        mcbot::FriendlyFire friendly_fire = (mcbot::FriendlyFire) friendly;
        std::string nametag_visibility = PacketDecoder::read_string(packet, offset);
        uint8_t color = PacketDecoder::read_byte(packet, offset);

        bot->log_debug(
            "Team Display Name: " + team_display_name +
            "\n\tTeam Prefix: " + team_prefix +
            "\n\tTeam Suffix: " + team_suffix +
            "\n\tFriendly Fire: " + StringUtils::to_string(friendly_fire) +
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

        bot->log_debug("Players (" + std::to_string(player_count) + "): ");

        for (auto player : players)
        {
            bot->log_debug("\t\t" + player);
        }
    }

}

void mcbot::PacketReceiver::recv_plugin_message(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutCustomPayload...");

    std::string plugin_channel = PacketDecoder::read_string(packet, offset);
    mcbot::Buffer<uint8_t> data = PacketDecoder::read_byte_array(length - offset, packet, offset);

    bot->log_debug(
        "Plugin Channel: " + plugin_channel +
        "\n\tData: " + data.to_string());
}

void mcbot::PacketReceiver::recv_server_difficulty(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutServerDifficulty...");

    mcbot::Difficulty difficulty = (mcbot::Difficulty) PacketDecoder::read_byte(packet, offset);

    bot->log_debug("Difficulty: " + StringUtils::to_string(difficulty));
}

void mcbot::PacketReceiver::recv_world_border(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutWorldBorder...");

    mcbot::WorldBorderAction action = (mcbot::WorldBorderAction) PacketDecoder::read_var_int(packet, offset);

    switch (action)
    {
    case mcbot::WorldBorderAction::SET_SIZE:
    {
        double radius = PacketDecoder::read_double(packet, offset);
        //this->world_border.set_radius(radius);
        break;
    }

    case mcbot::WorldBorderAction::LERP_SIZE:
    {
        double old_radius = PacketDecoder::read_double(packet, offset);
        double new_radius = PacketDecoder::read_double(packet, offset);
        long speed = PacketDecoder::read_var_long(packet, offset);
        //this->world_border.set_old_radius(old_radius);
        //this->world_border.set_new_radius(new_radius);
        //this->world_border.set_speed(speed);
        break;
    }

    case mcbot::WorldBorderAction::SET_CENTER:
    {
        double x = PacketDecoder::read_double(packet, offset);
        double z = PacketDecoder::read_double(packet, offset);
        //this->world_border.set_x(x);
        //this->world_border.set_z(z);
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

    case mcbot::WorldBorderAction::SET_WARNING_TIME:
    {
        int warning_time = PacketDecoder::read_var_int(packet, offset);
        //this->world_border.set_warning_time(warning_time);
        break;
    }

    case mcbot::WorldBorderAction::SET_WARNING_BLOCKS:
    {
        int warning_blocks = PacketDecoder::read_var_int(packet, offset);
        //this->world_border.set_warning_blocks(warning_blocks);
        break;
    }

    }

}

void mcbot::PacketReceiver::recv_title(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutTitle...");

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

    bot->log_debug("Action: " + StringUtils::to_string(action));
}

void mcbot::PacketReceiver::recv_player_list_header_footer(uint8_t* packet, size_t length, size_t& offset)
{
    bot->log_debug("<<< Handling PacketPlayOutPlayerListHeaderFooter...");

    std::string header = PacketDecoder::read_string(packet, offset);
    std::string footer = PacketDecoder::read_string(packet, offset);

    bot->log_debug(
        "Header: " + header +
        "\n\tFooter: " + footer);
}
