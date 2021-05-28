#pragma once

#include <iostream>
#include <array>

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

namespace mcbot
{
	class PacketDecoder
	{
	public:

		// Basic Types //
		static uint64_t ReadLong(uint8_t* packet, size_t& offset);
		static int64_t ReadVarLong(uint8_t* packet, size_t& offset);
		static uint32_t ReadInt(uint8_t* packet, size_t& offset);
		static int32_t ReadVarInt(uint8_t* packet, size_t& offset);
		static uint16_t ReadShort(uint8_t* packet, size_t& offset);
		static uint16_t ReadShortLittleEndian(uint8_t* packet, size_t& offset);
		static uint8_t ReadByte(uint8_t* bytes, size_t& offset);
		static uint8_t PeekByte(uint8_t* packet, size_t offset);
		static float ReadFloat(uint8_t* packet, size_t& offset);
		static double ReadDouble(uint8_t* packet, size_t& offset);
		static bool ReadBoolean(uint8_t* packet, size_t& offset);
		static std::string ReadString(uint8_t* packet, size_t& offset);
		static std::string ReadString(int length, uint8_t* packet, size_t& offset);


		// Classes //
		static UUID ReadUUID(uint8_t* packet, size_t& offset);
		static Slot ReadSlot(uint8_t* packet, size_t& offset);
		static Color ReadColor(uint8_t* packet, size_t& offset);
		static Particle ReadParticle(uint8_t* packet, size_t& offset);
		static Position ReadPosition(uint8_t* packet, size_t& offset);
		template<typename T> static Vector<T> ReadVector(uint8_t* packet, size_t& offset);
		static VillagerData ReadVillagerData(uint8_t* packet, size_t& offset);
		static AttributeModifier ReadAttributeModifier(uint8_t* packet, size_t& offset);
		static Attribute ReadAttribute(uint8_t* packet, size_t& offset);
		static EntityMetaData ReadMetaData(uint8_t* packet, size_t& offset);
		static Chunk ReadChunk(int x, int z, bool ground_up_continuous, bool sky_light_sent, uint16_t primary_bitmask, uint8_t* packet, size_t& offset);
		static Chunk ReadChunkBulk(Chunk& chunk, bool sky_light_sent, uint8_t* packet, size_t& offset);


		// NBT //
		static NBTTagCompound ReadNBT(uint8_t* packet, size_t& offset);
		static std::string ReadNBTString(uint8_t* packet, size_t& offset);
		static NBTTag ReadNextNBTTag(uint8_t* packet, size_t& offset);
		static NBTTag ReadNBTTag(NBTType type, uint8_t* packet, size_t& offset, bool has_name = true);
		static NBTTag ReadNBTTag(NBTType type, std::string name, uint8_t* packet, size_t& offset);
		static NBTTagCompound ReadNBTTagCompound(uint8_t* packet, size_t& offset, bool parent = false);
		static NBTList ReadNBTList(uint8_t* packet, size_t& offset);


		// Arrays //
		static void ReadByteArray(uint8_t* bytes, int length, uint8_t* packet, size_t& offset);
		static Buffer<uint8_t> ReadByteArray(int length, uint8_t* packet, size_t& offset);
		static Buffer<uint16_t> ReadShortArray(int length, uint8_t* packet, size_t& offset);
		static Buffer<uint16_t> ReadShortLittleEndianArray(int length, uint8_t* packet, size_t& offset); // little-endian
		static Buffer<int> ReadIntArray(int length, uint8_t* packet, size_t& offset);
		static Buffer<int> ReadVarIntArray(int length, uint8_t* packet, size_t& offset);
		static Buffer<long> ReadLongArray(int length, uint8_t* packet, size_t& offset);
		static std::list<std::string> ReadStringArray(int length, uint8_t* packet, size_t& offset);
		static std::list<Statistic> ReadStatisticArray(int length, uint8_t* packet, size_t& offset);
		static std::list<PlayerProperty> ReadPropertyArray(int length, uint8_t* packet, size_t& offset);
		static std::list<Slot> ReadSlotArray(int length, uint8_t* packet, size_t& offset);
	};

	template<typename T>
	inline Vector<T> PacketDecoder::ReadVector(uint8_t* packet, size_t& offset)
	{
		T x, y, z;
		if (std::is_same<T, float>())
		{
			x = ReadFloat(packet, offset);
			y = ReadFloat(packet, offset);
			z = ReadFloat(packet, offset);
		}
		else if (std::is_same<T, double>())
		{
			x = ReadDouble(packet, offset);
			y = ReadDouble(packet, offset);
			z = ReadDouble(packet, offset);
		}
		else if (std::is_same<T, uint8_t>() || std::is_same<T, int8_t>())
		{
			x = ReadByte(packet, offset);
			y = ReadByte(packet, offset);
			z = ReadByte(packet, offset);
		}
		else if (std::is_same<T, short>())
		{
			x = ReadShort(packet, offset);
			y = ReadShort(packet, offset);
			z = ReadShort(packet, offset);
		}
		else if (std::is_same<T, int>())
		{
			x = ReadInt(packet, offset);
			y = ReadInt(packet, offset);
			z = ReadInt(packet, offset);
		}
		else
		{
			std::cerr << "Unsupported vector type: " << typeid(T).name() << std::endl;
			return Vector<T>();
		}
		return Vector<T>(x, y, z);
	}
}

