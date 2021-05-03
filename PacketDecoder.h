#pragma once

#include <iostream>

#include "UUID.h"
#include "Slot.h"
#include "Color.h"
#include "Particle.h"
#include "Position.h"
#include "Vector.h"
#include "VillagerData.h"
#include "AttributeModifier.h"
#include "Attribute.h"
#include "NBT.h"
#include "Enums.h"
#include "EntityMetaData.h"
#include "Statistic.h"
#include "PlayerProperty.h"

namespace mcbot
{
	class PacketDecoder
	{

	public:

		// Read Standard Values
		static uint64_t read_long(uint8_t* packet, size_t& offset);
		static int64_t read_var_long(uint8_t* packet, size_t& offset);
		static uint32_t read_int(uint8_t* packet, size_t& offset);
		static int32_t read_var_int(uint8_t* packet, size_t& offset);
		static uint16_t read_short(uint8_t* packet, size_t& offset);
		static uint8_t read_byte(uint8_t* bytes, size_t& offset);
		static uint8_t peek_byte(uint8_t* packet, size_t offset);
		static float read_float(uint8_t* packet, size_t& offset);
		static double read_double(uint8_t* packet, size_t& offset);
		static bool read_boolean(uint8_t* packet, size_t& offset);
		static std::string read_string(uint8_t* packet, size_t& offset);
		static std::string read_string(int length, uint8_t* packet, size_t& offset);

		// Read Class Values
		static mcbot::UUID read_uuid(uint8_t* packet, size_t& offset);
		static mcbot::Slot read_slot(uint8_t* packet, size_t& offset);
		static mcbot::Color read_color(uint8_t* packet, size_t& offset);
		static mcbot::Particle read_particle(uint8_t* packet, size_t& offset);
		static mcbot::Position read_position(uint8_t* packet, size_t& offset);
		template<typename T>
		static mcbot::Vector<T> read_vector(uint8_t* packet, size_t& offset);
		static mcbot::VillagerData read_villager_data(uint8_t* packet, size_t& offset);
		static mcbot::AttributeModifier read_attribute_modifier(uint8_t* packet, size_t& offset);
		static mcbot::Attribute read_attribute(uint8_t* packet, size_t& offset);
		static mcbot::NBT read_nbt(uint8_t* packet, size_t& offset);
		static mcbot::NBT read_nbt_part(uint8_t* packet, size_t& offset, bool parent = true, bool list = false, mcbot::NBTType list_type = mcbot::NBTType::TAG_END);
		static mcbot::EntityMetaData read_meta_data(uint8_t* packet, size_t& offset);

		// Read Array Values
		static void read_byte_array(uint8_t* bytes, int length, uint8_t* packet, size_t& offset);
		static mcbot::Buffer<char> read_byte_array(int length, uint8_t* packet, size_t& offset);
		static mcbot::Buffer<int> read_int_array(int length, uint8_t* packet, size_t& offset);
		static mcbot::Buffer<int> read_var_int_array(int length, uint8_t* packet, size_t& offset);
		static mcbot::Buffer<long> read_long_array(int length, uint8_t* packet, size_t& offset);
		static std::list<std::string> read_string_array(int length, uint8_t* packet, size_t& offset);
		static std::list<mcbot::Statistic> read_statistic_array(int length, uint8_t* packet, size_t& offset);
		static std::list <mcbot::PlayerProperty> read_property_array(int length, uint8_t* packet, size_t& offset);
		static std::list <mcbot::Slot> read_slot_array(int length, uint8_t* packet, size_t& offset);
		static std::list<mcbot::NBT> read_nbt_list(int32_t length, mcbot::NBTType list_type, uint8_t* packet, size_t& offset);
	};

	template<typename T>
	inline mcbot::Vector<T> mcbot::PacketDecoder::read_vector(uint8_t* packet, size_t& offset)
	{
		T x, y, z;
		if (std::is_same<T, float>())
		{
			x = read_float(packet, offset);
			y = read_float(packet, offset);
			z = read_float(packet, offset);
		}
		else if (std::is_same<T, double>())
		{
			x = read_double(packet, offset);
			y = read_double(packet, offset);
			z = read_double(packet, offset);
		}
		else if (std::is_same<T, uint8_t>())
		{
			x = read_byte(packet, offset);
			y = read_byte(packet, offset);
			z = read_byte(packet, offset);
		}
		else if (std::is_same<T, short>())
		{
			x = read_short(packet, offset);
			y = read_short(packet, offset);
			z = read_short(packet, offset);
		}
		else if (std::is_same<T, int>())
		{
			x = read_int(packet, offset);
			y = read_int(packet, offset);
			z = read_int(packet, offset);
		}
		else
		{
			std::cerr << "Unsupported vector type: " << typeid(T).name() << std::endl;
		}
		return mcbot::Vector<T>(x, y, z);
	}
}
