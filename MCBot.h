#pragma once

#include "Socket.h"
#include "Buffer.h"

#include "MojangSession.h"

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
#include "NBTTagCompound.h"
#include "Particle.h"
#include "WorldBorder.h"
#include "Position.h"
#include "Vector.h"

namespace mcbot
{
	class PacketReceiver;
	class PacketSender;

	class MCBot
	{
	private:

		// Configuration
		bool debug;

		// User info
		MojangSession session;

		// Connection Info
		Socket sock;
		bool connected;
		bool ready;
		State state;
		PacketReceiver* packet_receiver;
		PacketSender* packet_sender;

		// Runtime Minecraft Info
		EntityPlayer player;
		std::map<UUID, EntityPlayer> uuid_to_player;
		std::map<int, Entity> entities;
		std::map<std::pair<int, int>, Chunk> chunks;
		WorldBorder world_border;
		
	public:
		MCBot();
		~MCBot();

		int connect_server(char* hostname, char* port);

		// High-level interaction methods
		void move(mcbot::Vector<double> diff);
		void move(double dx, double dz);
		void move_to(mcbot::Vector<double> destination, double speed, bool ignore_ground);
		void move_to(double x, double z, int ticks_per_move);
		void move_to_ground(double speed);
		bool on_ground();
		mcbot::Vector<double> get_ground_location(mcbot::Vector<double> location);
		void attack_entity(Entity entity);

		// Entity Management
		void register_entity(Entity entity);
		void remove_entity(int id);
		bool is_entity_registered(int id);
		void register_player(UUID uuid, EntityPlayer player);
		EntityPlayer& get_player(UUID uuid);
		Entity& get_entity(int id);
		bool is_player_registered(UUID uuid);
		void update_player_info(PlayerInfoAction action, int players_length, uint8_t* packet, size_t& offset);

		// Chunk Management
		void load_chunk(Chunk chunk);
		Chunk& get_chunk(int x, int z);
		Chunk& get_chunk(mcbot::Vector<int> location);
		Chunk& get_chunk(mcbot::Vector<double> location);

		// Logging
		void log_debug(std::string message);
		void log_error(std::string message);
		void log_info(std::string message);
		void log_chat(std::string message);

		void set_debug(bool debug);
		void set_state(State state);
		void set_connected(bool connected);
		void set_session(MojangSession session);
		MojangSession get_session();

		std::list<Entity> get_entities();
		std::list<EntityPlayer> get_players();
		bool is_connected();
		bool is_ready();
		State get_state();
		Socket& get_socket();
		PacketSender& get_packet_sender();
		PacketReceiver& get_packet_receiver();
		EntityPlayer& get_player();
	};
}

