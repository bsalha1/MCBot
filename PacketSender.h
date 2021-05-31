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


		// HTTP Mojang Requests //
		int LoginMojang(std::string email, std::string password);
		int VerifyAccessToken();
		int SendSession(std::string server_id, std::string shared_secret, uint8_t* public_key, int public_key_length);


		// TCP Server Outgoing Requests //
		void SendHandshake(char* hostname, unsigned short port);
		void SendLoginStart();
		void SendEncryptionResponse(uint8_t* public_key, int public_key_length, uint8_t* verify_token, int verify_token_length, std::string shared_secret);
		void SendKeepAlive(int id);
		void SendChatMessage(std::string message);
		void SendUseEntity(int entity_id, UseEntityType action);
		void SendPosition(Vector<double> position, bool on_ground);
		void SendLook(float yaw, float pitch, bool on_ground);
		void SendPositionLook(Vector<double> position, float yaw, float pitch, bool on_ground);
		void SendBlockDig(DigStatus status, Vector<int> location, BlockFace face);
		void SendBlockPlace(Vector<int> location, BlockFace face, Slot held_item, Vector<uint8_t> cursor_position);
		void SendHeldItemSlot(short slot);
		void SendArmAnimation();
		void SendEntityAction(EntityAction action, int action_parameter = 0);
		void SendEntityAction(int player_id, EntityAction action, int param);
		void SendSetCreativeSlot(short slot, Slot item);
		void SendEnchantItem(uint8_t window_id, uint8_t enchantment);
		void SendUpdateSign(Vector<int> location, std::string line1, std::string line2, std::string line3, std::string line4);
		void SendAbilities(uint8_t flags, float flying_speed, float walking_speed);
		void SendTabComplete(std::string text, bool has_position, Vector<int> block_position = Vector<int>());
		void SendSettings();
		void SendClientCommand(ClientStatus status);
		void SendCustomPayload(std::string message);
	};
}

