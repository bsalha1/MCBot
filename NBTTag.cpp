#include "NBTTag.h"

mcbot::NBTTag::NBTTag(NBTType type, std::string name, boost::any value)
{
	this->type = type;
	this->name = name;
	this->value = value;
}

mcbot::NBTTag::NBTTag(NBTType type, std::string name)
{
	this->type = type;
	this->name = name;
}

mcbot::NBTTag::NBTTag()
{
	this->type = NBTType::UNKNOWN;
	this->name = "NONE";
}

std::string mcbot::NBTTag::get_name()
{
	return this->name;
}

boost::any mcbot::NBTTag::get_value()
{
	return this->value;
}