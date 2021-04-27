#pragma once

#include <iostream>
#include <map>
#include <list>
#include <typeinfo>

#include "Buffer.h"

namespace mcbot
{
	class NBT
	{
	private:
		int pretty_pointer = 0;
		std::map<std::string, char> bytes;
		std::map<std::string, short> shorts;
		std::map<std::string, int> ints;
		std::map<std::string, long> longs;
		std::map<std::string, float> floats;
		std::map<std::string, double> doubles;
		std::map<std::string, mcbot::Buffer<char>> byte_arrays;
		std::map<std::string, mcbot::Buffer<int>> int_arrays;
		std::map<std::string, mcbot::Buffer<long>> long_arrays;
		std::map<std::string, std::string> strings;
		std::map<std::string, std::list<mcbot::NBT>> nbt_lists;
		std::map<std::string, mcbot::NBT> nbt_compounds;

	public:
		NBT();

		void add_byte(std::string name, char field);
		void add_short(std::string name, short field);
		void add_int(std::string name, int field);
		void add_long(std::string name, long field);
		void add_float(std::string name, float field);
		void add_double(std::string name, double field);
		void add_byte_array(std::string name, mcbot::Buffer<char> field);
		void add_int_array(std::string name, mcbot::Buffer<int> field);
		void add_long_array(std::string name, mcbot::Buffer<long> field);
		void add_string(std::string name, std::string field);
		void add_nbt_list(std::string name, std::list<mcbot::NBT> field);
		void add_nbt(std::string name, mcbot::NBT field);

		std::string to_string();

	};
}

