#include "NBTTagCompound.h"
#include "Enums.h"

#include <typeinfo>

mcbot::NBTTagCompound::NBTTagCompound()
{
}

void mcbot::NBTTagCompound::add_tag(mcbot::NBTTag tag)
{
	this->tags.push_back(tag);
}

void mcbot::NBTTagCompound::serialize(uint8_t* packet, size_t& offset)
{
	for (auto tag : this->tags)
	{
		tag.serialize(packet, offset);
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
std::string mcbot::NBTTagCompound::to_string()
{
	std::string str = "";

	/*std::string spaces = insert_spaces(pretty_pointer);

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
	}*/

	return str;
}

