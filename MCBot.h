#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <iostream>

#include "lib/httplib.h"
#include <openssl/rsa.h>

namespace mcbot
{
	class MCBot
	{
	private:
		std::string email;
		std::string password;
		std::string username;
		std::string access_token;
		std::string uuid;
		std::string server_id;
		std::string shared_secret;

		// Encryption
		const unsigned char* public_key;
		const unsigned char* verify_token;
		size_t public_key_length;
		size_t verify_token_length;

		SOCKET sock;

		// Packet parsers
		static int read_var_int(char* bytes, size_t* offset);
		static int read_string_n(char* string, size_t string_length, char* packet, size_t packet_size, size_t* offset);
		static void read_byte_array(char* bytes, int bytes_length, char* packet, size_t packet_size, size_t* offset);
		static void read_byte_array(unsigned char* bytes, int bytes_length, char* packet, size_t packet_size, size_t* offset);
		static void write_var_int(int value, char* packet, size_t packet_size, size_t* offset);
		static size_t get_var_int_size(int value);
		static void write_byte_array(unsigned char* bytes, int bytes_length, char* packet, size_t packet_size, size_t* offset);
		static void write_string_n(char* string, char* packet, size_t packet_size, size_t* offset);
		static void write_ushort(unsigned short num, char* packet, size_t packet_size, size_t* offset);
		static void write_packet_length(char* packet, size_t packet_size, size_t* offset);

	public:
		MCBot(std::string email, std::string password);
		~MCBot();

		// HTTP Mojang requests
		int login_mojang();
		int verify_access_token();
		int save_session();

		// Server outgoing requests
		int connect_server(char* hostname, char* port);
		void send_handshake(char* hostname, unsigned short port);
		void send_login_start();
		void send_encryption_request();

		// Server incoming requests
		void recv_packet();
		void recv_encryption_request(char* packet, size_t size_read, size_t* offset);
	};
}

