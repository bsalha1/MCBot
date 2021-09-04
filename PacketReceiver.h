#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
#include <boost/algorithm/string.hpp>

#include "Socket.h"
#include "Buffer.h"
#include "PacketEncoder.h"
#include "PacketDecoder.h"
#include "Packet.h"

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

namespace McBot
{
	class MCBot;
	
	class PacketReceiver
	{
	private:
		MCBot* bot;
		bool compression_enabled;

	public:

		// Constructors //
		PacketReceiver(MCBot* bot);
		PacketReceiver();


		/*
			Reads next packet and passes it to the corresponding callback function to parse it.
		*/
		void ReadAndHandleNextPacket();
		
		
		/*
			Reads next VarInt in the stream.
			@return returns the value of the VarInt
		*/
		int ReadNextVarInt();


		/*
			Reads next packet in the stream. 
			First reads size and uncompressed size (if compression is enabled) then passes that to the socket to receive.
			@return received packet object
		*/
		Packet ReadNextPacket();


		/*
			Parses packet by calling the corresponding callback function.
			@param packet_id: ID of the packet to identify which function to handle the packet
			@param packet: the packet object to parse
		*/
		void HandlePacket(int packet_id, Packet packet);


		// Status: LOGIN //
		void RecvEncryptionRequest(Packet packet);
		void RecvSetCompression(Packet packet);
		void RecvLoginSuccess(Packet packet);
		void RecvLoginDisconnect(Packet packet);


		// Status: PLAY //
		void RecvPlayDisconnect(Packet packet);
		void RecvKeepAlive(Packet packet);
		void RecvJoinServer(Packet packet);
		void RecvChatMessage(Packet packet);
		void RecvUpdateTime(Packet packet);
		void RecvEntityEquipment(Packet packet);
		void RecvSpawnPosition(Packet packet);
		void RecvUpdateHealth(Packet packet);
		void RecvPosition(Packet packet);
		void RecvHeldItemSlot(Packet packet);
		void RecvBed(Packet packet);
		void RecvAnimation(Packet packet);
		void RecvNamedEntitySpawn(Packet packet);
		void RecvCollect(Packet packet);
		void RecvSpawnEntity(Packet packet);
		void RecvSpawnEntityLiving(Packet packet);
		void RecvSpawnEntityPainting(Packet packet);
		void RecvSpawnEntityExperienceOrb(Packet packet);
		void RecvEntityVelocity(Packet packet);
		void RecvEntityDestroy(Packet packet);
		void RecvEntity(Packet packet);
		void RecvRelEntityMove(Packet packet);
		void RecvEntityLook(Packet packet);
		void RecvRelEntityMoveLook(Packet packet);
		void RecvEntityTeleport(Packet packet);
		void RecvEntityHeadLook(Packet packet);
		void RecvEntityStatus(Packet packet);
		void RecvEntityMetadata(Packet packet);
		void RecvEntityEffect(Packet packet);
		void RecvExperience(Packet packet);
		void RecvEntityAttributes(Packet packet);
		void RecvMapChunk(Packet packet);
		void RecvMultiBlockChange(Packet packet);
		void RecvBlockChange(Packet packet);
		void RecvBlockBreakAnimation(Packet packet);
		void RecvPluginMessage(Packet packet);
		void RecvMapChunkBulk(Packet packet);
		void RecvWorldEvent(Packet packet);
		void RecvNamedSoundEffect(Packet packet);
		void RecvWorldParticles(Packet packet);
		void RecvGameStateChange(Packet packet);
		void RecvSpawnEntityWeather(Packet packet);
		void RecvSetSlot(Packet packet);
		void RecvWindowItems(Packet packet);
		void RecvTransaction(Packet packet);
		void RecvUpdateSign(Packet packet);
		void RecvTileEntityData(Packet packet);
		void RecvStatistics(Packet packet);
		void RecvPlayerInfo(Packet packet);
		void RecvAbilities(Packet packet);
		void RecvScoreboardTeam(Packet packet);
		void RecvScoreboardObjective(Packet packet);
		void RecvUpdateScoreboardScore(Packet packet);
		void RecvDisplayScoreboard(Packet packet);
		void RecvServerDifficulty(Packet packet);
		void RecvWorldBorder(Packet packet);
		void RecvTitle(Packet packet);
		void RecvPlayerListHeaderFooter(Packet packet);
	};
}

