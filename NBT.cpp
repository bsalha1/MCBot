#include "NBT.h"
#include "Enums.h"

mcbot::NBT::NBT()
{
}

void mcbot::NBT::add_byte(std::string name, int8_t field)
{
	this->tags.push_back(NBTTag(NBTType::TAG_BYTE, name, field));
}

void mcbot::NBT::add_short(std::string name, int16_t field)
{
	this->tags.push_back(NBTTag(NBTType::TAG_SHORT, name, field));
}

void mcbot::NBT::add_int(std::string name, int32_t field)
{
	this->tags.push_back(NBTTag(NBTType::TAG_INT, name, field));
}

void mcbot::NBT::add_long(std::string name, int64_t field)
{
	this->tags.push_back(NBTTag(NBTType::TAG_LONG, name, field));
}

void mcbot::NBT::add_float(std::string name, float field)
{
	this->tags.push_back(NBTTag(NBTType::TAG_FLOAT, name, field));
}

void mcbot::NBT::add_double(std::string name, double field)
{
	this->tags.push_back(NBTTag(NBTType::TAG_DOUBLE, name, field));
}

void mcbot::NBT::add_byte_array(std::string name, mcbot::Buffer<int8_t> field)
{
	this->tags.push_back(NBTTag(NBTType::TAG_BYTE_ARRAY, name, field));
}

void mcbot::NBT::add_int_array(std::string name, mcbot::Buffer<int32_t> field)
{
	this->tags.push_back(NBTTag(NBTType::TAG_INT_ARRAY, name, field));
}

void mcbot::NBT::add_long_array(std::string name, mcbot::Buffer<int64_t> field)
{
	this->long_arrays.insert(std::pair<std::string, mcbot::Buffer<int64_t>>(name, field));
}

void mcbot::NBT::add_string(std::string name, std::string field)
{
	this->strings.insert(std::pair<std::string, std::string>(name, field));
}

void mcbot::NBT::add_nbt_list(std::string name, std::list<mcbot::NBT> field)
{
	this->nbt_lists.insert(std::pair<std::string, std::list<mcbot::NBT>>(name, field));
}

void mcbot::NBT::add_nbt(std::string name, mcbot::NBT field)
{
	this->nbt_compounds.insert(std::pair<std::string, mcbot::NBT>(name, field));
}

void mcbot::NBT::serialize(uint8_t* packet, size_t& offset)
{
	for (auto pair : this->bytes)
	{
		PacketEncoder::write_byte((uint8_t)NBTType::TAG_BYTE, packet, offset);
		PacketEncoder::write_nbt_string(pair.first, packet, offset);
		PacketEncoder::write_byte(pair.second, packet, offset);
	}

	for (auto pair : this->shorts)
	{
		PacketEncoder::write_byte((uint8_t)NBTType::TAG_SHORT, packet, offset);
		PacketEncoder::write_nbt_string(pair.first, packet, offset);
		PacketEncoder::write_short(pair.second, packet, offset);
	}

	for (auto pair : this->ints)
	{
		PacketEncoder::write_byte((uint8_t)NBTType::TAG_INT, packet, offset);
		PacketEncoder::write_nbt_string(pair.first, packet, offset);
		PacketEncoder::write_int(pair.second, packet, offset);
	}

	for (auto pair : this->longs)
	{
		PacketEncoder::write_byte((uint8_t)NBTType::TAG_LONG, packet, offset);
		PacketEncoder::write_nbt_string(pair.first, packet, offset);
		PacketEncoder::write_long(pair.second, packet, offset);
	}

	for (auto pair : this->floats)
	{
		PacketEncoder::write_byte((uint8_t)NBTType::TAG_FLOAT, packet, offset);
		PacketEncoder::write_nbt_string(pair.first, packet, offset);
		PacketEncoder::write_float(pair.second, packet, offset);
	}

	for (auto pair : this->doubles)
	{
		PacketEncoder::write_byte((uint8_t)NBTType::TAG_DOUBLE, packet, offset);
		PacketEncoder::write_nbt_string(pair.first, packet, offset);
		PacketEncoder::write_double(pair.second, packet, offset);
	}

	for (auto pair : this->byte_arrays)
	{
		PacketEncoder::write_byte((uint8_t)NBTType::TAG_BYTE_ARRAY, packet, offset);
		PacketEncoder::write_nbt_string(pair.first, packet, offset);
		PacketEncoder::write_byte_array((uint8_t*)pair.second.get_array(), pair.second.get_current_size(), packet, offset);
	}

	for (auto pair : this->int_arrays)
	{
		PacketEncoder::write_byte((uint8_t)NBTType::TAG_INT_ARRAY, packet, offset);
		PacketEncoder::write_nbt_string(pair.first, packet, offset);
		PacketEncoder::write_int_array((uint32_t*)pair.second.get_array(), pair.second.get_current_size(), packet, offset);
	}

	for (auto pair : this->long_arrays)
	{
		PacketEncoder::write_byte((uint8_t)NBTType::TAG_LONG_ARRAY, packet, offset);
		PacketEncoder::write_nbt_string(pair.first, packet, offset);
		PacketEncoder::write_long_array((uint64_t*)pair.second.get_array(), pair.second.get_current_size(), packet, offset);
	}

	for (auto pair : this->strings)
	{
		PacketEncoder::write_byte((uint8_t)NBTType::TAG_STRING, packet, offset);
		PacketEncoder::write_nbt_string(pair.first, packet, offset);
		PacketEncoder::write_nbt_string(pair.second, packet, offset);
	}

	for (auto pair : this->nbt_lists)
	{
		PacketEncoder::write_byte((uint8_t)NBTType::TAG_LIST, packet, offset);
		PacketEncoder::write_nbt_string(pair.first, packet, offset);
		for (auto nbt : pair.second)
		{
			nbt.serialize(packet, offset);
		}
	}

	for (auto pair : this->nbt_compounds)
	{
		PacketEncoder::write_byte((uint8_t)NBTType::TAG_COMPOUND, packet, offset);
		PacketEncoder::write_nbt_string(pair.first, packet, offset);
		pair.second.serialize(packet, offset);
		PacketEncoder::write_byte((uint8_t)NBTType::TAG_END, packet, offset);
	}
}

std::string insert_spaces(int num_spaces)
{
	std::string str = "";

	for (int i = 0; i < num_spaces; i++)
	{
		str += " ";
	}

	return str;
}
std::string mcbot::NBT::to_string()
{
	std::string str = "";

	std::string spaces = insert_spaces(pretty_pointer);

	for (auto pair : bytes)
	{
		str += spaces + "TAG_Byte(" + pair.first + "):" + std::to_string(pair.second) + "\n";
	}

	for (auto pair : shorts)
	{
		str += spaces + "TAG_Short(" + pair.first + "): " + std::to_string(pair.second) + "\n";
	}

	for (auto pair : ints)
	{
		str += spaces + "TAG_Int(" + pair.first + "): " + std::to_string(pair.second) + "\n";
	}

	for (auto pair : longs)
	{
		str += spaces + "TAG_Long(" + pair.first + "): " + std::to_string(pair.second) + "\n";
	}

	for (auto pair : floats)
	{
		str += spaces + "TAG_Float(" + pair.first + "): " + std::to_string(pair.second) + "\n";
	}

	for (auto pair : doubles)
	{
		str += spaces + "TAG_Double(" + pair.first + "): " + std::to_string(pair.second) + "\n";
	}

	for (auto pair : strings)
	{
		str += spaces + "TAG_String(" + pair.first + "): \"" + pair.second + "\"\n";
	}

	for (auto pair : this->nbt_lists)
	{
		str += spaces + "TAG_List(" + pair.first + "):\n" + spaces + "{\n";

		for (auto nbt : pair.second)
		{
			nbt.pretty_pointer += this->pretty_pointer + 4;
			str += nbt.to_string() + "\n";
			nbt.pretty_pointer -= (this->pretty_pointer + 4);
		}

		str += spaces + "}\n";
	}

	for (auto pair : this->nbt_compounds)
	{
		pair.second.pretty_pointer += this->pretty_pointer + 4;
		str += spaces + "TAG_Compound(" + pair.first + "):\n" + spaces + "{\n" + pair.second.to_string() + "\n" + spaces + "}\n";
		pair.second.pretty_pointer -= (this->pretty_pointer + 4);
	}

	for (auto pair : this->byte_arrays)
	{
		str += "TAG_Byte_Array(" + pair.first + "): " + pair.second.to_string() + "\n";
	}

	for (auto pair : this->int_arrays)
	{
		str += "TAG_Int_Array(" + pair.first + "): " + pair.second.to_string() + "\n";
	}

	for (auto pair : this->long_arrays)
	{
		str += "TAG_Long_Array(" + pair.first + "): " + pair.second.to_string() + "\n";
	}

	return str;
}

