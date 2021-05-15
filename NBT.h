#pragma once

#include <iostream>
#include <map>
#include <list>
#include <typeinfo>
#include <boost/any.hpp>

#include "NBTTag.h"
#include "NBTList.h"
#include "PacketEncoder.h"
#include "Buffer.h"
#include "Serializable.h"

namespace mcbot
{
	class NBT : Serializable
	{
	private:
		int pretty_pointer = 0;
		std::list<NBTTag> tags;

		std::list<NBTList<boost::any>> nbt_lists;

	public:
		NBT();

		void add_byte(std::string name, int8_t field);
		void add_short(std::string name, int16_t field);
		void add_int(std::string name, int32_t field);
		void add_long(std::string name, int64_t field);
		void add_float(std::string name, float field);
		void add_double(std::string name, double field);
		void add_byte_array(std::string name, mcbot::Buffer<int8_t> field);
		void add_int_array(std::string name, mcbot::Buffer<int32_t> field);
		void add_long_array(std::string name, mcbot::Buffer<int64_t> field);
		void add_string(std::string name, std::string field);
		void add_nbt_list(std::string name, std::list<mcbot::NBT> field);
		void add_nbt(std::string name, NBT field);

		void serialize(uint8_t* packet, size_t& offset) override;

		std::string to_string();

	};
}

