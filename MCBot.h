#pragma once

#include "Socket.h"
#include "Buffer.h"
#include "Packet.h"
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
	// Declared here due to circular dependency
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


		/*
			Connect local socket to server
			@param hostname: The hostname of the server to connect to
			@param port: The port of the server to connect to
			@return -1 if unsuccessful, 0 otherwise
		*/
		int ConnectToServer(char* hostname, char* port);


		/*
			Handshake with server and then send PacketPlayInLoginStart to server
			@param hostname: The hostname of the server to log in to
			@param port: The port of the server to log in to
		*/
		void LoginToServer(char* hostname, char* port);


		/*
			Log in to Mojang
			@param email: Email of user to log in to
			@param password: Password of user to log in to
			@return -1 if unsuccessful, 0 otherwise
		*/
		int LoginToMojang(char* email, char* password);


		/*
			Receive packets and dispatch the necessary tasks continually until `connected = false`
			@return thread object
		*/
		std::thread StartPacketReceiverThread();


		/*
			Send player's current position, yaw and pitch every tick
			@return thread object
		*/
		std::thread StartPositionThread();

		
		/*
			Send packet via "this->socket" 
			@param packet: packet to send
			@return number of bytes sent
		*/
		int SendPacket(Packet packet);


		/*
			Initialize socket to encrypt and decrypt on sending and receiving
			@param key: the key to use in encryption
			@param iv: the initial value to use in encryption
		*/
		void InitEncryption(uint8_t* key, uint8_t* iv);


		/*
			Initialize socket to compress and decompress on sending and receiving
			@param max_uncompressed_length: if sending packet greater than this size, compress it, otherwise do not compress it
		*/
		void InitCompression(int max_uncompressed_length);


		// High-Level Interaction Methods //
		void Move(Vector<double> diff);
		void Move(double dx, double dz);
		void MoveTo(Vector<double> destination, double speed);
		void MoveTo(double x, double z, int ms_per_block);
		void MoveToGround(double speed);
		bool OnGround();
		Vector<double> GetGroundLocation(Vector<double> location);
		void AttackEntity(Entity entity);


		// Registries //
		Registry<int, Entity>& GetEntityRegistry();
		Registry<UUID, EntityPlayer>& GetPlayerRegistry();
		Registry<std::pair<int, int>, Chunk>& GetChunkRegistry();

		void UpdatePlayerInfo(PlayerInfoAction action, int players_length, Packet& packet);
		void UpdatePlayerLocation(Vector<double> location);
		void UpdatePlayerRotation(float yaw, float pitch);
		void UpdatePlayerInventory(std::array<Slot, 45> player_inventory);


		// Chunk Management //
		Chunk& GetChunk(int x, int z);
		Chunk& GetChunk(Vector<int> location);
		Chunk& GetChunk(Vector<double> location);
		std::list<Chunk> GetChunks(int x, int z, int radius);


		// Setters //
		void SetState(State state);
		void SetConnected(bool connected);
		void SetSession(MojangSession session);


		// Variable Access //
		MojangSession GetSession() const;
		bool IsConnected() const;
		State GetState() const;
		Socket GetSocket();
		PacketSender& GetPacketSender();
		PacketReceiver& GetPacketReceiver();
		Logger& GetLogger();
		EntityPlayer GetPlayer() const;
	};
}

