#pragma once

#include <iostream>

#include "Slot.h"

namespace mcbot
{
	class PacketEncoder
	{
	public:
		static void write_var_int(int value, uint8_t* packet, size_t& offset);
		static int get_var_int_size(int value);
		static int get_var_int_size(std::initializer_list<int> values);
		static void write_byte_array(uint8_t* bytes, int bytes_length, uint8_t* packet, size_t& offset);
		static void write_int_array(uint32_t* ints, int ints_length, uint8_t* packet, size_t& offset);
		static void write_long_array(uint64_t* longs, int longs_length, uint8_t* packet, size_t& offset);
		static void write_string(char* string, uint8_t* packet, size_t& offset);
		static void write_string(std::string string, uint8_t* packet, size_t& offset);
		static void write_nbt_string(std::string string, uint8_t* packet, size_t& offset);
		static void write_boolean(bool value, uint8_t* packet, size_t& offset);
		static void write_byte(uint8_t num, uint8_t* packet, size_t& offset);
		static void write_short(uint16_t num, uint8_t* packet, size_t& offset);
		static void write_int(uint32_t num, uint8_t* packet, size_t& offset);
		static void write_long(uint64_t num, uint8_t* packet, size_t& offset);
		static void write_double(double num, uint8_t* packet, size_t& offset);
		static void write_float(float num, uint8_t* packet, size_t& offset);
		static void write_position(int x, int y, int z, uint8_t* packet, size_t& offset);
		static void write_slot(Slot slot, uint8_t* packet, size_t& offset);
		static void write_nbt(mcbot::NBT nbt, uint8_t* packet, size_t& offset);
	};
}

