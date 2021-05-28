#pragma once

#include <iostream>

#include "Slot.h"

namespace mcbot
{
	class PacketEncoder
	{
	public:

		// VarInt //
		static void WriteVarInt(int value, uint8_t* packet, size_t& offset);
		static int GetVarIntNumBytes(int value);
		static int GetVarIntNumBytes(std::initializer_list<int> values);


		// Arrays //
		static void WriteByteArray(uint8_t* bytes, int bytes_length, uint8_t* packet, size_t& offset);
		static void WriteIntArray(uint32_t* ints, int ints_length, uint8_t* packet, size_t& offset);
		static void WriteLongArray(uint64_t* longs, int longs_length, uint8_t* packet, size_t& offset);


		// Strings //
		static void WriteString(char* string, uint8_t* packet, size_t& offset);
		static void WriteString(std::string string, uint8_t* packet, size_t& offset);
		static void WriteNBTString(std::string string, uint8_t* packet, size_t& offset);


		// Basic Types //
		static void WriteBoolean(bool value, uint8_t* packet, size_t& offset);
		static void WriteByte(uint8_t num, uint8_t* packet, size_t& offset);
		static void WriteShort(uint16_t num, uint8_t* packet, size_t& offset);
		static void WriteInt(uint32_t num, uint8_t* packet, size_t& offset);
		static void WriteLong(uint64_t num, uint8_t* packet, size_t& offset);
		static void WriteDouble(double num, uint8_t* packet, size_t& offset);
		static void WriteFloat(float num, uint8_t* packet, size_t& offset);


		// Classes //
		static void WritePosition(int x, int y, int z, uint8_t* packet, size_t& offset);
		static void WriteSerializable(Serializable const& serializable, uint8_t* packet, size_t& offset);
	};
}

