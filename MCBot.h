#pragma once

#include "Socket.h"
#include "Enums.h"
#include "Location.h"
#include "Statistic.h"

namespace mcbot
{
	class MCBot
	{
	private:

		bool connected;
		mcbot::State state;

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

		mcbot::Socket sock;

		// Packet parsers
		static int read_var_int(uint8_t* packet, size_t &offset);
		static int read_int(uint8_t* packet, size_t& offset);
		static uint16_t read_ushort(uint8_t* packet, size_t& offset);
		static int8_t read_byte(uint8_t* bytes, size_t& offset);
		static uint8_t read_ubyte(uint8_t* bytes, size_t& offset);
		static float read_float(uint8_t* packet, size_t& offset);
		static bool read_boolean(uint8_t* packet, size_t& offset);
		static std::string read_string(uint8_t* packet, size_t &offset);
		static void read_byte_array(uint8_t* bytes, int bytes_length, uint8_t* packet, size_t &offset);
		static std::list<std::string> read_string_array(int strings_length, uint8_t* packet, size_t& offset);
		static std::list<mcbot::Statistic> read_statistic_array(int statistics_length, uint8_t* packet, size_t& offset);
		static mcbot::Location read_location(uint8_t* packet, size_t& offset);

		static void write_var_int(int value, uint8_t* packet, size_t packet_size, size_t &offset);
		static size_t get_var_int_size(int value);
		static void write_byte_array(uint8_t* bytes, int bytes_length, uint8_t* packet, size_t packet_size, size_t &offset);
		static void write_string_n(char* string, uint8_t* packet, size_t packet_size, size_t &offset);
		static void write_ushort(unsigned short num, uint8_t* packet, size_t packet_size, size_t &offset);
		static void write_packet_length(uint8_t* packet, size_t packet_size, size_t &offset);

	public:
		MCBot(std::string email, std::string password);
		~MCBot();

		// HTTP Mojang requests
		int login_mojang();
		int verify_access_token();
		int send_session();

		// Server outgoing requests
		int connect_server(char* hostname, char* port);
		void send_handshake(char* hostname, unsigned short port);
		void send_login_start();
		void send_encryption_response();
		void send_message(char* message);

		int read_next_var_int();
		void read_next_packet(int length, uint8_t* packet);

		// Server incoming requests
		void handle_recv_packet(int packet_id, uint8_t* packet, int bytes_read, size_t& offset);
		void recv_packet();
		void recv_disconnect(uint8_t* packet, size_t size_read, size_t& offset);
		
		// LOGIN
		void recv_encryption_request(uint8_t* packet, size_t size_read, size_t &offset);
		void recv_set_compression(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_login_success(uint8_t* packet, size_t size_read, size_t& offset);

		// PLAY
		void recv_join_server(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_spawn_position(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_held_item_slot(uint8_t* packet, size_t length, size_t& offset);
		void recv_plugin_message(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_map_chunk_bulk(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_statistics(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_abilities(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_scoreboard_team(uint8_t* packet, size_t length, size_t& offset);
		void recv_server_difficulty(uint8_t* packet, size_t size_read, size_t& offset);

		bool is_connected();
		bool is_encrypted();
	};
}

