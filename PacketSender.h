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
		void send_use_entity(int entity_id, UseEntityType action);
		void send_position(Vector<double> position, bool on_ground);
		void send_look(float yaw, float pitch, bool on_ground);
		void send_position_look(Vector<double> position, float yaw, float pitch, bool on_ground);
		void send_block_dig(DigStatus status, mcbot::Vector<int> location, BlockFace face);
		void send_block_place(mcbot::Vector<int> location, BlockFace face, Slot held_item, mcbot::Vector<uint8_t> cursor_position);
		void send_held_item_slot(short slot);
		void send_arm_animation();
		void send_entity_action(EntityAction action, int action_parameter = 0);
		void send_entity_action(int player_id, EntityAction action, int param);
		void send_set_creative_slot(short slot, Slot item);
		void send_enchant_item(uint8_t window_id, uint8_t enchantment);
		void send_update_sign(mcbot::Vector<int> location, std::string line1, std::string line2, std::string line3, std::string line4);
		void send_abilities(uint8_t flags, float flying_speed, float walking_speed);
		void send_tab_complete(std::string text, bool has_position, mcbot::Vector<int> block_position = mcbot::Vector<int>());
		void send_settings();
		void send_client_command(ClientStatus status);
		void send_custom_payload(std::string message);
	};
}

