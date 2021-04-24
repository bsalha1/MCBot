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

void mcbot::NBT::add_byte_array(std::string name, mcbot::Buffer<char> field)
{
	this->byte_arrays.insert(std::pair<std::string, mcbot::Buffer<char>>(name, field));
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

