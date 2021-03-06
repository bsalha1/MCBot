#pragma once

#include <iostream>
#include <array>

#include "Reflection.h"

#include "Packet.h"

#include "Chunk.h"
#include "UUID.h"
#include "Slot.h"
#include "Color.h"
#include "Particle.h"
#include "Position.h"
#include "Vector.h"
#include "VillagerData.h"
#include "AttributeModifier.h"
#include "Attribute.h"
#include "NBTTagCompound.h"
#include "NBTList.h"
#include "Enums.h"
#include "EntityMetaData.h"
#include "Statistic.h"
#include "PlayerProperty.h"

namespace McBot
{
	class PacketDecoder
	{
	public:

		// Basic Types //
		static uint64_t ReadLong(Packet& packet);
		static int64_t ReadVarLong(Packet& packet);
		static uint32_t ReadInt(Packet& packet);
		static int32_t ReadVarInt(Packet& packet);
		static uint16_t ReadShort(Packet& packet);
		static uint16_t ReadShortLittleEndian(Packet& packet);
		static uint8_t ReadByte(Packet& packet);
		static uint8_t PeekByte(Packet& packet);
		static float ReadFloat(Packet& packet);
		static double ReadDouble(Packet& packet);
		static bool ReadBoolean(Packet& packet);
		static std::string ReadString(Packet& packet);
		static std::string ReadString(int length, Packet& packet);


		// Classes //
		static UUID ReadUUID(Packet& packet);
		static Slot ReadSlot(Packet& packet);
		static Color ReadColor(Packet& packet);
		static Particle ReadParticle(Packet& packet);
		static Position ReadPosition(Packet& packet);
		template<typename T> static Vector<T> ReadVector(Packet& packet);
		static VillagerData ReadVillagerData(Packet& packet);
		static AttributeModifier ReadAttributeModifier(Packet& packet);
		static Attribute ReadAttribute(Packet& packet);
		static EntityMetaData ReadMetaData(Packet& packet);
		static Chunk ReadChunk(int x, int z, bool ground_up_continuous, bool sky_light_sent, uint16_t primary_bitmask, Packet& packet);
		static Chunk ReadChunkBulk(Chunk& chunk, bool sky_light_sent, Packet& packet);


		// NBT //
		static NBTTagCompound ReadNBT(Packet& packet);
		static std::string ReadNBTString(Packet& packet);
		static NBTTag ReadNextNBTTag(Packet& packet);
		static NBTTag ReadNBTTag(NBTType type, Packet& packet, bool has_name = true);
		static NBTTag ReadNBTTag(NBTType type, std::string name, Packet& packet);
		static NBTTagCompound ReadNBTTagCompound(Packet& packet, bool parent = false);
		static NBTList ReadNBTList(Packet& packet);


		// Arrays //
		static void ReadByteArray(uint8_t* bytes, int length, Packet& packet);
		static Buffer<uint8_t> ReadByteArray(size_t length, Packet& packet);
		static Buffer<uint16_t> ReadShortArray(int length, Packet& packet);
		static Buffer<uint16_t> ReadShortLittleEndianArray(int length, Packet& packet); // little-endian
		static Buffer<int> ReadIntArray(int length, Packet& packet);
		static Buffer<int> ReadVarIntArray(int length, Packet& packet);
		static Buffer<long> ReadLongArray(int length, Packet& packet);
		static std::list<std::string> ReadStringArray(int length, Packet& packet);
		static std::list<Statistic> ReadStatisticArray(int length, Packet& packet);
		static std::list<PlayerProperty> ReadPropertyArray(int length, Packet& packet);
		static std::list<Slot> ReadSlotArray(int length, Packet& packet);
	};

	template<typename T>
	inline Vector<T> PacketDecoder::ReadVector(Packet& packet)
	{
		if (INSTANCE_OF(T, float))
		{
			float x = ReadFloat(packet);
			float y = ReadFloat(packet);
			float z = ReadFloat(packet);
			return Vector<T>(x, y, z);
		}
		else if (INSTANCE_OF(T, double))
		{
			double x = ReadDouble(packet);
			double y = ReadDouble(packet);
			double z = ReadDouble(packet);
			return Vector<T>(x, y, z);
		}
		else if (INSTANCE_OF(T, uint8_t) || INSTANCE_OF(T, int8_t))
		{
			uint8_t x = ReadByte(packet);
			uint8_t y = ReadByte(packet);
			uint8_t z = ReadByte(packet);
			return Vector<T>(x, y, z);
		}
		else if (INSTANCE_OF(T, short))
		{
			short x = ReadShort(packet);
			short y = ReadShort(packet);
			short z = ReadShort(packet);
			return Vector<T>(x, y, z);
		}
		else if (INSTANCE_OF(T, int))
		{
			int x = ReadInt(packet);
			int y = ReadInt(packet);
			int z = ReadInt(packet);
			return Vector<T>(x, y, z);
		}
		else
		{
			std::cerr << "Unsupported vector type: " << typeid(T).name() << std::endl;
			return Vector<T>();
		}
	}
}

