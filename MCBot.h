#pragma once

#include "Socket.h"
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0) 

namespace mcbot
{
	class MCBot
	{
	private:

		bool connected;

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
		size_t public_key_length;
		size_t verify_token_length;

		// Compression
		bool compression_enabled;
		int max_packet_size;

		mcbot::Socket sock;

		// Packet parsers
		static int read_var_int(uint8_t* packet, size_t &offset);
		static int read_int(uint8_t* packet, size_t& offset);
		static unsigned short read_ushort(uint8_t* packet, size_t& offset);
		static bool read_boolean(uint8_t* packet, size_t& offset);
		static int read_string_n(char* string, size_t string_length, uint8_t* packet, size_t packet_size, size_t &offset);
		static void read_byte_array(uint8_t* bytes, int bytes_length, uint8_t* packet, size_t packet_size, size_t &offset);\

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
		void recv_encryption_request(uint8_t* packet, size_t size_read, size_t &offset);
		void recv_set_compression(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_login_success(uint8_t* packet, size_t size_read, size_t& offset);
		void recv_map_chunk_bulk(uint8_t* packet, size_t size_read, size_t& offset);

		bool is_connected();
		bool is_encrypted();
	};
}

