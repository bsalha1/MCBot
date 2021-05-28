#pragma once

#include "Socket.h"
#include "Buffer.h"
#include "Logger.h"
#include "Vector.h"
#include "Registry.h"

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

#define TPS 20 // Ticks per second

namespace mcbot
{
	class PacketReceiver;
	class PacketSender;

	class MCBot
	{
	private:

		Logger logger;


		// User info
		MojangSession session;


		// Connection Info
		Socket sock;
		bool connected;
		State state;
		PacketReceiver* packet_receiver;
		PacketSender* packet_sender;


		// Runtime Minecraft Info
		EntityPlayer player;
		WorldBorder world_border;
		Registry<int, Entity> entity_registry;
		Registry<UUID, EntityPlayer> player_registry;
		Registry<std::pair<int, int>, Chunk> chunk_registry;
		
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


		// Registries //
		Registry<int, Entity>& GetEntityRegistry();
		Registry<UUID, EntityPlayer>& GetPlayerRegistry();
		Registry<std::pair<int, int>, Chunk>& GetChunkRegistry();

		void UpdatePlayerInfo(PlayerInfoAction action, int players_length, uint8_t* packet, size_t& offset);


		// Chunk Management //
		Chunk& GetChunk(int x, int z);
		Chunk& GetChunk(Vector<int> location);
		Chunk& GetChunk(Vector<double> location);
		std::list<Chunk> GetChunks(int x, int z, unsigned int radius);


		// Setters //
		void SetState(State state);
		void SetConnected(bool connected);
		void SetSession(MojangSession session);


		// Variable Access //
		MojangSession GetSession();
		bool IsConnected();
		State GetState();
		Socket& GetSocket();
		PacketSender& GetPacketSender();
		PacketReceiver& GetPacketReceiver();
		Logger& GetLogger();
		EntityPlayer& GetPlayer();
	};
}

