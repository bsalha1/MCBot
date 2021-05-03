#pragma once

#include "Socket.h"
#include "Buffer.h"

#include "VillagerData.h"
#include "EntityMetadata.h"
#include "Attribute.h"
#include "Color.h"
#include "Enums.h"
#include "Slot.h"
#include "Statistic.h"
#include "Player.h"
#include "UUID.h"
#include "NBT.h"
#include "Particle.h"
#include "WorldBorder.h"
#include "Position.h"
#include "Vector.h"

namespace mcbot
{
	class MCBot
	{
	private:
		// Configuration
		bool debug;

		// User info
		std::string email;
		std::string password;
		std::string username;
		std::string access_token;
		std::string uuid;
		std::string server_id;
		std::string shared_secret;


		// Encryption
		bool encryption_enabled;
		uint8_t* public_key;
		uint8_t* verify_token;
		int public_key_length;
		int verify_token_length;


		// Compression
		bool compression_enabled;
		int max_uncompressed_length;


		// Connection Info
		mcbot::Socket sock;
		bool connected;
		mcbot::State state;

		// Runtime Minecraft Info
		std::map<mcbot::UUID, mcbot::Player> uuid_to_player;
		mcbot::WorldBorder world_border;

		// Read Standard Values
		static uint64_t read_long(uint8_t* packet, size_t& offset);
		static int64_t read_var_long(uint8_t* packet, size_t& offset);
		static uint32_t read_int(uint8_t* packet, size_t& offset);
		static int32_t read_var_int(uint8_t* packet, size_t& offset);
		static uint16_t read_short(uint8_t* packet, size_t& offset);
		static uint8_t read_byte(uint8_t* bytes, size_t& offset);
		static uint8_t peek_byte(uint8_t* packet, size_t offset);
		static float read_float(uint8_t* packet, size_t& offset);
		static double read_double(uint8_t* packet, size_t& offset);
		static bool read_boolean(uint8_t* packet, size_t& offset);
		static std::string read_string(uint8_t* packet, size_t& offset);
		static std::string read_string(int length, uint8_t* packet, size_t& offset);

		// Read Class Values
		static mcbot::UUID read_uuid(uint8_t* packet, size_t& offset);
		static mcbot::Slot read_slot(uint8_t* packet, size_t& offset);
		static mcbot::Color read_color(uint8_t* packet, size_t& offset);
		static mcbot::Particle read_particle(uint8_t* packet, size_t& offset);
		static mcbot::Position read_position(uint8_t* packet, size_t& offset);
		template<typename T>
		static mcbot::Vector<T> read_vector(uint8_t* packet, size_t& offset);
		static mcbot::VillagerData read_villager_data(uint8_t* packet, size_t& offset);
		static mcbot::AttributeModifier read_attribute_modifier(uint8_t* packet, size_t& offset);
		static mcbot::Attribute read_attribute(uint8_t* packet, size_t& offset);
		static mcbot::NBT read_nbt(uint8_t* packet, size_t& offset);
		static mcbot::NBT read_nbt_part(uint8_t* packet, size_t& offset, bool parent = true, bool list = false, mcbot::NBTType list_type = mcbot::NBTType::TAG_END);
		static mcbot::EntityMetaData read_meta_data(uint8_t* packet, size_t& offset);

		// Read Array Values
		static void read_byte_array(uint8_t* bytes, int length, uint8_t* packet, size_t& offset);
		static mcbot::Buffer<char> read_byte_array(int length, uint8_t* packet, size_t& offset);
		static mcbot::Buffer<int> read_int_array(int length, uint8_t* packet, size_t& offset);
		static mcbot::Buffer<int> read_var_int_array(int length, uint8_t* packet, size_t& offset);
		static mcbot::Buffer<long> read_long_array(int length, uint8_t* packet, size_t& offset);
		static std::list<std::string> read_string_array(int length , uint8_t* packet, size_t& offset);
		static std::list<mcbot::Statistic> read_statistic_array(int length, uint8_t* packet, size_t& offset);
		static std::list <mcbot::PlayerProperty> read_property_array(int length, uint8_t* packet, size_t& offset);
		static std::list <mcbot::Slot> read_slot_array(int length, uint8_t* packet, size_t& offset);
		static std::list<mcbot::NBT> read_nbt_list(int32_t length, mcbot::NBTType list_type, uint8_t* packet, size_t& offset);

		static void write_var_int(int value, uint8_t* packet, size_t& offset);
		static size_t get_var_int_size(int value);
		static void write_byte_array(uint8_t* bytes, int bytes_length, uint8_t* packet, size_t& offset);
		static void write_string(char* string, uint8_t* packet, size_t& offset);
		static void write_string(std::string string, uint8_t* packet, size_t& offset);
		static void write_ushort(unsigned short num, uint8_t* packet, size_t& offset);
		
		void update_player_info(mcbot::PlayerInfoAction action, int players_length, uint8_t* packet, size_t& offset);

	public:
		MCBot(std::string email, std::string password);
		~MCBot();

		// Logging
		void log_debug(std::string message);
		void log_error(std::string message);
		void log_info(std::string message);
		void log_chat(std::string message);

		void set_debug(bool debug);

		// HTTP Mojang requests
		int login_mojang();
		int verify_access_token();
		int send_session();

		// Server outgoing requests
		int connect_server(char* hostname, char* port);
		void send_handshake(char* hostname, unsigned short port);
		void send_login_start();
		void send_encryption_response();
		void send_keep_alive(int id);
		void send_chat_message(std::string message);

		int read_next_var_int();
		int read_next_packet(int length, uint8_t* packet, int decompressed_length = 0);

		// Server incoming requests
		void handle_recv_packet(int packet_id, uint8_t* packet, int bytes_read, size_t& offset);
		void recv_packet();
		void recv_login_disconnect(uint8_t* packet, size_t size_read, size_t& offset);
		
		// LOGIN
		void recv_encryption_request(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_set_compression(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_login_success(uint8_t* packet, size_t size_read, size_t& offset);

		// PLAY
		void recv_play_disconnect(uint8_t* packet, size_t length, size_t& offset);
		void recv_keep_alive(uint8_t* packet, size_t length, size_t& offset);
		void recv_join_server(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_chat_message(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_update_time(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_equipment(uint8_t* packet, size_t length, size_t& offset);
		void recv_spawn_position(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_update_health(uint8_t* packet, size_t length, size_t& offset);
		void recv_position(uint8_t* packet, size_t length, size_t& offset);
		void recv_held_item_slot(uint8_t* packet, size_t length, size_t& offset);
		void recv_bed(uint8_t* packet, size_t length, size_t& offset);
		void recv_animation(uint8_t* packet, size_t length, size_t& offset);
		void recv_named_entity_spawn(uint8_t* packet, size_t length, size_t& offset);
		void recv_collect(uint8_t* packet, size_t length, size_t& offset);
		void recv_spawn_entity(uint8_t* packet, size_t length, size_t& offset);
		void recv_spawn_entity_living(uint8_t* packet, size_t length, size_t& offset);
		void recv_spawn_entity_painting(uint8_t* packet, size_t length, size_t& offset);
		void recv_spawn_entity_experience_orb(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_velocity(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_destroy(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity(uint8_t* packet, size_t length, size_t& offset);
		void recv_rel_entity_move(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_look(uint8_t* packet, size_t length, size_t& offset);
		void recv_rel_entity_move_look(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_teleport(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_head_look(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_status(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_metadata(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_effect(uint8_t* packet, size_t length, size_t& offset);
		void recv_experience(uint8_t* packet, size_t length, size_t& offset);
		void recv_entity_attributes(uint8_t* packet, size_t length, size_t& offset);
		void recv_map_chunk(uint8_t* packet, size_t length, size_t& offset);
		void recv_block_change(uint8_t* packet, size_t length, size_t& offset);
		void recv_block_break(uint8_t* packet, size_t length, size_t& offset);
		void recv_plugin_message(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_map_chunk_bulk(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_world_event(uint8_t* packet, size_t length, size_t& offset);
		void recv_named_sound_effect(uint8_t* packet, size_t length, size_t& offset);
		void recv_world_particles(uint8_t* packet, size_t length, size_t& offset);
		void recv_game_state_change(uint8_t* packet, size_t length, size_t& offset);
		void recv_set_slot(uint8_t* packet, size_t length, size_t& offset);
		void recv_window_items(uint8_t* packet, size_t length, size_t& offset);
		void recv_transaction(uint8_t* packet, size_t length, size_t& offset);
		void recv_update_sign(uint8_t* packet, size_t length, size_t& offset);
		void recv_tile_entity_data(uint8_t* packet, size_t length, size_t& offset);
		void recv_statistics(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_player_info(uint8_t* packet, size_t length, size_t& offset);
		void recv_abilities(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_scoreboard_team(uint8_t* packet, size_t length, size_t& offset);
		void recv_scoreboard_objective(uint8_t* packet, size_t length, size_t& offset);
		void recv_update_scoreboard_score(uint8_t* packet, size_t length, size_t& offset);
		void recv_display_scoreboard(uint8_t* packet, size_t length, size_t& offset);
		void recv_server_difficulty(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_world_border(uint8_t* packet, size_t length, size_t& offset);
		void recv_title(uint8_t* packet, size_t length, size_t& offset);
		void recv_player_list_header_footer(uint8_t* packet, size_t size_read, size_t& offset);

		bool is_connected();
		bool is_encrypted();
	};
}

