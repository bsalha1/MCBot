#pragma once

#include "Socket.h"
#include "Buffer.h"

#include "CollisionException.h"

#include "Chunk.h"
#include "Block.h"
#include "VillagerData.h"
#include "Entity.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "EntityMetadata.h"
#include "Attribute.h"
#include "Color.h"
#include "Enums.h"
#include "Slot.h"
#include "Statistic.h"
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
		Socket sock;
		bool connected;
		bool ready;
		State state;
		EntityPlayer player;

		// Runtime Minecraft Info
		std::map<UUID, EntityPlayer> uuid_to_player;
		std::list<Entity> entities;
		std::map<std::pair<int, int>, Chunk> chunks;
		WorldBorder world_border;
		
		void update_player_info(PlayerInfoAction action, int players_length, uint8_t* packet, size_t& offset);

	public:
		MCBot(std::string email, std::string password);
		~MCBot();

		// High-level methods
		void move(mcbot::Vector<double> diff);
		void move(double dx, double dz);
		void move_to(mcbot::Vector<double> destination, double speed, bool ignore_ground);
		void move_to(double x, double z, int speed);
		void move_to_ground(double speed);
		bool on_ground();
		mcbot::Vector<double> get_ground_location(mcbot::Vector<double> location);
		void load_chunk(Chunk chunk);
		Chunk get_chunk(int x, int z);
		Chunk get_chunk(mcbot::Vector<double> location);

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
		void send_position(Vector<double> position, bool on_ground);
		void send_look(float yaw, float pitch, bool on_ground);
		void send_position_look(Vector<double> position, float yaw, float pitch, bool on_ground);
		void send_held_item_slot(short slot);
		void send_settings();
		void send_custom_payload(std::string message);

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
		void recv_multi_block_change(uint8_t* packet, size_t length, size_t& offset);
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

		std::list<Entity> get_entities();
		bool is_connected();
		bool is_ready();
		bool is_encrypted();
		EntityPlayer get_player();
	};
}

