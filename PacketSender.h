#pragma once

#include <nlohmann/json.hpp>

#include "PacketEncoder.h"

#include "Vector.h"
#include "DaftHash.h"
#include "MojangSession.h"
#include "base64.h"

namespace mcbot
{
	class MCBot;

	class PacketSender
	{
	private:
		MCBot* bot;

	public:
		PacketSender(MCBot* bot);
		PacketSender();

		// HTTP Mojang requests
		int login_mojang(std::string email, std::string password);
		int verify_access_token();
		int send_session(std::string server_id, std::string shared_secret, uint8_t* public_key, int public_key_length);

		// Server outgoing requests
		void send_handshake(char* hostname, unsigned short port);
		void send_login_start();
		void send_encryption_response(uint8_t* public_key, int public_key_length, uint8_t* verify_token, int verify_token_length, std::string shared_secret);
		void send_keep_alive(int id);
		void send_chat_message(std::string message);
		void send_position(Vector<double> position, bool on_ground);
		void send_look(float yaw, float pitch, bool on_ground);
		void send_position_look(Vector<double> position, float yaw, float pitch, bool on_ground);
		void send_held_item_slot(short slot);
		void send_settings();
		void send_custom_payload(std::string message);
	};
}

