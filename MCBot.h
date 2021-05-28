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

#define TPS 20 // Ticks per second

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

		int ConnectToServer(char* hostname, char* port);


		// High-Level Interaction Methods //
		void Move(Vector<double> diff);
		void Move(double dx, double dz);
		void MoveTo(Vector<double> destination, double speed, bool ignore_ground);
		void MoveTo(double x, double z, int ticks_per_move);
		void MoveToGround(double speed);
		bool OnGround();
		Vector<double> GetGroundLocation(Vector<double> location);
		void AttackEntity(Entity entity);


		// Entity Management //
		void RegisterEntity(Entity entity);
		void RemoveEntity(int id);
		bool IsEntityRegistered(int id);
		Entity& GetEntity(int id);


		// Player Management //
		void RegisterPlayer(UUID uuid, EntityPlayer player);
		void RemovePlayer(EntityPlayer player);
		void RemovePlayer(UUID uuid);
		bool IsPlayerRegistered(UUID uuid);
		EntityPlayer& GetPlayer(UUID uuid);
		void UpdatePlayerInfo(PlayerInfoAction action, int players_length, uint8_t* packet, size_t& offset);


		// Chunk Management //
		void LoadChunk(Chunk chunk);
		Chunk& GetChunk(int x, int z);
		Chunk& GetChunk(Vector<int> location);
		Chunk& GetChunk(Vector<double> location);
		std::list<Chunk> GetChunks(int x, int z, unsigned int radius);


		// Logging //
		void LogDebug(std::string message);
		void LogError(std::string message);
		void LogInfo(std::string message);
		void LogChat(std::string message);


		// Configuration //
		void SetDebug(bool debug);
		void SetState(State state);
		void SetConnected(bool connected);
		void SetSession(MojangSession session);

		// Variable Access //
		MojangSession GetSession();
		std::list<Entity> GetEntities();
		std::list<EntityPlayer> GetPlayers();
		bool IsConnected();
		bool is_ready();
		State GetState();
		Socket& GetSocket();
		PacketSender& GetPacketSender();
		PacketReceiver& GetPacketReceiver();
		EntityPlayer& GetPlayer();
	};
}

