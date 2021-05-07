#include "NBT.h"

mcbot::NBT::NBT()
{
}

void mcbot::NBT::add_byte(std::string name, char field)
{
	this->bytes.insert(std::pair<std::string, char>(name, field));
}

void mcbot::NBT::add_short(std::string name, short field)
{
	this->shorts.insert(std::pair<std::string, short>(name, field));
}

void mcbot::NBT::add_int(std::string name, int field)
{
	this->ints.insert(std::pair<std::string, int>(name, field));
}

void mcbot::NBT::add_long(std::string name, long field)
{
	this->longs.insert(std::pair<std::string, long>(name, field));
}

void mcbot::NBT::add_float(std::string name, float field)
{
	this->floats.insert(std::pair<std::string, float>(name, field));
}

void mcbot::NBT::add_double(std::string name, double field)
{
	this->doubles.insert(std::pair<std::string, double>(name, field));
}

void mcbot::NBT::add_byte_array(std::string name, mcbot::Buffer<uint8_t> field)
{
	this->byte_arrays.insert(std::pair<std::string, mcbot::Buffer<uint8_t>>(name, field));
}

void mcbot::NBT::add_int_array(std::string name, mcbot::Buffer<int> field)
{
	this->int_arrays.insert(std::pair<std::string, mcbot::Buffer<int>>(name, field));
}

void mcbot::NBT::add_long_array(std::string name, mcbot::Buffer<long> field)
{
	this->long_arrays.insert(std::pair<std::string, mcbot::Buffer<long>>(name, field));
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

