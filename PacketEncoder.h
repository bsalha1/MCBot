#pragma once

#include <iostream>

namespace mcbot
{
	class PacketEncoder
	{
	public:
		static void write_var_int(int value, uint8_t* packet, size_t& offset);
		static size_t get_var_int_size(int value);
		static void write_byte_array(uint8_t* bytes, int bytes_length, uint8_t* packet, size_t& offset);
		static void write_string(char* string, uint8_t* packet, size_t& offset);
		static void write_string(std::string string, uint8_t* packet, size_t& offset);
		static void write_ushort(unsigned short num, uint8_t* packet, size_t& offset);
	};
}

