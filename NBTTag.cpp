#include "NBTTag.h"
#include "NBTList.h"
#include "PacketEncoder.h"
#include "Buffer.h"

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

void mcbot::NBTTag::serialize(uint8_t* packet, size_t& offset)
{
	PacketEncoder::write_byte((uint8_t)this->type, packet, offset);
	PacketEncoder::write_nbt_string(this->name, packet, offset);

	switch (this->type)
	{
	case NBTType::TAG_BYTE:
		PacketEncoder::write_byte(boost::any_cast<int8_t>(this->value), packet, offset);
		break;
	case NBTType::TAG_SHORT:
		PacketEncoder::write_short(boost::any_cast<int16_t>(this->value), packet, offset);
		break;
	case NBTType::TAG_INT:
		PacketEncoder::write_int(boost::any_cast<int32_t>(this->value), packet, offset);
		break;
	case NBTType::TAG_LONG:
		PacketEncoder::write_long(boost::any_cast<int64_t>(this->value), packet, offset);
		break;
	case NBTType::TAG_FLOAT:
		PacketEncoder::write_float(boost::any_cast<float>(this->value), packet, offset);
		break;
	case NBTType::TAG_DOUBLE:
		PacketEncoder::write_double(boost::any_cast<double>(this->value), packet, offset);
		break;
	case NBTType::TAG_STRING:
		PacketEncoder::write_string(boost::any_cast<std::string>(this->value), packet, offset);
		break;
	case NBTType::TAG_BYTE_ARRAY:
	{
		mcbot::Buffer<int8_t> arr = boost::any_cast<mcbot::Buffer<int8_t>>(this->value);
		PacketEncoder::write_int((uint32_t)arr.get_current_size(), packet, offset);
		PacketEncoder::write_byte_array((uint8_t*)arr.get_array(), arr.get_current_size(), packet, offset);
		break;
	}
	case NBTType::TAG_INT_ARRAY:
	{
		mcbot::Buffer<int32_t> arr = boost::any_cast<mcbot::Buffer<int32_t>>(this->value);
		PacketEncoder::write_int((uint32_t)arr.get_current_size(), packet, offset);
		PacketEncoder::write_int_array((uint32_t*)arr.get_array(), arr.get_current_size(), packet, offset);
		break;
	}
	case NBTType::TAG_LONG_ARRAY:
	{
		mcbot::Buffer<int64_t> arr = boost::any_cast<mcbot::Buffer<int64_t>>(this->value);
		PacketEncoder::write_int((uint32_t)arr.get_current_size(), packet, offset);
		PacketEncoder::write_long_array((uint64_t*)arr.get_array(), arr.get_current_size(), packet, offset);
		break;
	}
	case NBTType::TAG_LIST:
	{
		NBTList nbt_list = boost::any_cast<NBTList>(this->value);
		PacketEncoder::write_byte((uint8_t)nbt_list.get_type(), packet, offset);
		PacketEncoder::write_int((uint32_t)nbt_list.get_elements().size(), packet, offset);
		for (NBTTag tag : nbt_list.get_elements())
		{
			tag.serialize(packet, offset);
		}
		break;
	}
	case NBTType::TAG_COMPOUND:
	{
		NBTTagCompound nbt_compound = boost::any_cast<NBTTagCompound>(this->value);
		nbt_compound.serialize(packet, offset);
		break;
	}
	}
}

std::string mcbot::NBTTag::get_name()
{
	return this->name;
}

mcbot::NBTType mcbot::NBTTag::get_type()
{
	return this->type;
}

boost::any mcbot::NBTTag::get_value()
{
	return this->value;
}