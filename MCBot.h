#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "lib/httplib.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

namespace mcbot
{
	class MCBot
	{
	private:
		const std::string auth_url = "https://authserver.mojang.com";
		std::string email;
		std::string password;
		std::string username;
		std::string access_token;
		SOCKET sock;

		// Packet parsers
		static int read_var_int(char* bytes, size_t* offset);
		static int read_string_n(char* string, size_t string_length, char* packet, size_t packet_size, size_t* offset);
		static void read_byte_array(char* bytes, int bytes_length, char* packet, size_t packet_size, size_t* offset);
		static void write_var_int(int value, char* packet, size_t packet_size, size_t* offset);
		static size_t get_var_int_size(int value);
		static void write_string_n(char* string, char* packet, size_t packet_size, size_t* offset);
		static void write_ushort(unsigned short num, char* packet, size_t packet_size, size_t* offset);
		static void write_packet_length(char* packet, size_t packet_size, size_t* offset);

	public:
		MCBot(std::string email, std::string password);

		int mojang_login();

		int connect_server(char* hostname, char* port);

		void send_handshake(char* hostname, unsigned short port);
		void send_login_start();

		void recv_packet();
		void recv_encryption_request(char* packet, size_t size_read, size_t* offset);
	};
}

