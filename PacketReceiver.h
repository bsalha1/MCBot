#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
#include <boost/algorithm/string.hpp>

#include "Socket.h"
#include "Buffer.h"
#include "PacketEncoder.h"
#include "PacketDecoder.h"

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
	class MCBot;
	
	class PacketReceiver
	{
	private:
		MCBot* bot;
		EntityPlayer* player;
		bool compression_enabled;

	public:

		// Constructors //
		PacketReceiver(MCBot* bot);
		PacketReceiver();


		// Server incoming requests //
		void RecvPacket();
		int ReadNextVarInt();
		int ReadNextPacket(int length, uint8_t* packet, int decompressed_length = 0);
		void HandleRecvPacket(int packet_id, uint8_t* packet, int bytes_read, size_t& offset);


		// Status: LOGIN //
		void RecvEncryptionRequest(uint8_t* packet, size_t size_read, size_t& offset);
		void RecvSetCompression(uint8_t* packet, size_t size_read, size_t& offset);
		void RecvLoginSuccess(uint8_t* packet, size_t size_read, size_t& offset);
		void RecvLoginDisconnect(uint8_t* packet, size_t size_read, size_t& offset);


		// Status: PLAY //
		void RecvPlayDisconnect(uint8_t* packet, size_t length, size_t& offset);
		void RecvKeepAlive(uint8_t* packet, size_t length, size_t& offset);
		void RecvJoinServer(uint8_t* packet, size_t size_read, size_t& offset);
		void RecvChatMessage(uint8_t* packet, size_t size_read, size_t& offset);
		void RecvUpdateTime(uint8_t* packet, size_t length, size_t& offset);
		void RecvEntityEquipment(uint8_t* packet, size_t length, size_t& offset);
		void RecvSpawnPosition(uint8_t* packet, size_t size_read, size_t& offset);
		void RecvUpdateHealth(uint8_t* packet, size_t length, size_t& offset);
		void RecvPosition(uint8_t* packet, size_t length, size_t& offset);
		void RecvHeldItemSlot(uint8_t* packet, size_t length, size_t& offset);
		void RecvBed(uint8_t* packet, size_t length, size_t& offset);
		void RecvAnimation(uint8_t* packet, size_t length, size_t& offset);
		void RecvNamedEntitySpawn(uint8_t* packet, size_t length, size_t& offset);
		void RecvCollect(uint8_t* packet, size_t length, size_t& offset);
		void RecvSpawnEntity(uint8_t* packet, size_t length, size_t& offset);
		void RecvSpawnEntityLiving(uint8_t* packet, size_t length, size_t& offset);
		void RecvSpawnEntityPainting(uint8_t* packet, size_t length, size_t& offset);
		void RecvSpawnEntityExperienceOrb(uint8_t* packet, size_t length, size_t& offset);
		void RecvEntityVelocity(uint8_t* packet, size_t length, size_t& offset);
		void RecvEntityDestroy(uint8_t* packet, size_t length, size_t& offset);
		void RecvEntity(uint8_t* packet, size_t length, size_t& offset);
		void RecvRelEntityMove(uint8_t* packet, size_t length, size_t& offset);
		void RecvEntityLook(uint8_t* packet, size_t length, size_t& offset);
		void RecvRelEntityMoveLook(uint8_t* packet, size_t length, size_t& offset);
		void RecvEntityTeleport(uint8_t* packet, size_t length, size_t& offset);
		void RecvEntityHeadLook(uint8_t* packet, size_t length, size_t& offset);
		void RecvEntityStatus(uint8_t* packet, size_t length, size_t& offset);
		void RecvEntityMetadata(uint8_t* packet, size_t length, size_t& offset);
		void RecvEntityEffect(uint8_t* packet, size_t length, size_t& offset);
		void RecvExperience(uint8_t* packet, size_t length, size_t& offset);
		void RecvEntityAttributes(uint8_t* packet, size_t length, size_t& offset);
		void RecvMapChunk(uint8_t* packet, size_t length, size_t& offset);
		void RecvMultiBlockChange(uint8_t* packet, size_t length, size_t& offset);
		void RecvBlockChange(uint8_t* packet, size_t length, size_t& offset);
		void RecvBlockBreakAnimation(uint8_t* packet, size_t length, size_t& offset);
		void RecvPluginMessage(uint8_t* packet, size_t size_read, size_t& offset);
		void RecvMapChunkBulk(uint8_t* packet, size_t size_read, size_t& offset);
		void RecvWorldEvent(uint8_t* packet, size_t length, size_t& offset);
		void RecvNamedSoundEffect(uint8_t* packet, size_t length, size_t& offset);
		void RecvWorldParticles(uint8_t* packet, size_t length, size_t& offset);
		void RecvGameStateChange(uint8_t* packet, size_t length, size_t& offset);
		void RecvSpawnEntityWeather(uint8_t* packet, size_t length, size_t& offset);
		void RecvSetSlot(uint8_t* packet, size_t length, size_t& offset);
		void RecvWindowItems(uint8_t* packet, size_t length, size_t& offset);
		void RecvTransaction(uint8_t* packet, size_t length, size_t& offset);
		void RecvUpdateSign(uint8_t* packet, size_t length, size_t& offset);
		void RecvTileEntityData(uint8_t* packet, size_t length, size_t& offset);
		void RecvStatistics(uint8_t* packet, size_t size_read, size_t& offset);
		void RecvPlayerInfo(uint8_t* packet, size_t length, size_t& offset);
		void RecvAbilities(uint8_t* packet, size_t size_read, size_t& offset);
		void RecvScoreboardTeam(uint8_t* packet, size_t length, size_t& offset);
		void RecvScoreboardObjective(uint8_t* packet, size_t length, size_t& offset);
		void RecvUpdateScoreboardScore(uint8_t* packet, size_t length, size_t& offset);
		void RecvDisplayScoreboard(uint8_t* packet, size_t length, size_t& offset);
		void RecvServerDifficulty(uint8_t* packet, size_t size_read, size_t& offset);
		void RecvWorldBorder(uint8_t* packet, size_t length, size_t& offset);
		void RecvTitle(uint8_t* packet, size_t length, size_t& offset);
		void RecvPlayerListHeaderFooter(uint8_t* packet, size_t size_read, size_t& offset);
	};
}

