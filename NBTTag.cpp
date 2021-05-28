#include "NBTTag.h"
#include "NBTList.h"
#include "PacketEncoder.h"
#include "Buffer.h"

namespace mcbot
{

	NBTTag::NBTTag(NBTType type, std::string name, boost::any value)
	{
		this->type = type;
		this->name = name;
		this->value = value;
	}

	NBTTag::NBTTag(NBTType type, std::string name)
	{
		this->type = type;
		this->name = name;
	}

	NBTTag::NBTTag()
	{
		this->type = NBTType::UNKNOWN;
		this->name = "NONE";
	}

	void NBTTag::Serialize(uint8_t* packet, size_t& offset) const
	{
		PacketEncoder::WriteByte((uint8_t)this->type, packet, offset);
		PacketEncoder::WriteNBTString(this->name, packet, offset);

		switch (this->type)
		{
		case NBTType::TAG_BYTE:
			PacketEncoder::WriteByte(boost::any_cast<int8_t>(this->value), packet, offset);
			break;
		case NBTType::TAG_SHORT:
			PacketEncoder::WriteShort(boost::any_cast<int16_t>(this->value), packet, offset);
			break;
		case NBTType::TAG_INT:
			PacketEncoder::WriteInt(boost::any_cast<int32_t>(this->value), packet, offset);
			break;
		case NBTType::TAG_LONG:
			PacketEncoder::WriteLong(boost::any_cast<int64_t>(this->value), packet, offset);
			break;
		case NBTType::TAG_FLOAT:
			PacketEncoder::WriteFloat(boost::any_cast<float>(this->value), packet, offset);
			break;
		case NBTType::TAG_DOUBLE:
			PacketEncoder::WriteDouble(boost::any_cast<double>(this->value), packet, offset);
			break;
		case NBTType::TAG_STRING:
			PacketEncoder::WriteString(boost::any_cast<std::string>(this->value), packet, offset);
			break;
		case NBTType::TAG_BYTE_ARRAY:
		{
			Buffer<int8_t> arr = boost::any_cast<Buffer<int8_t>>(this->value);
			PacketEncoder::WriteInt((uint32_t)arr.get_current_size(), packet, offset);
			PacketEncoder::WriteByteArray((uint8_t*)arr.get_array(), arr.get_current_size(), packet, offset);
			break;
		}
		case NBTType::TAG_INT_ARRAY:
		{
			Buffer<int32_t> arr = boost::any_cast<Buffer<int32_t>>(this->value);
			PacketEncoder::WriteInt((uint32_t)arr.get_current_size(), packet, offset);
			PacketEncoder::WriteIntArray((uint32_t*)arr.get_array(), arr.get_current_size(), packet, offset);
			break;
		}
		case NBTType::TAG_LONG_ARRAY:
		{
			Buffer<int64_t> arr = boost::any_cast<Buffer<int64_t>>(this->value);
			PacketEncoder::WriteInt((uint32_t)arr.get_current_size(), packet, offset);
			PacketEncoder::WriteLongArray((uint64_t*)arr.get_array(), arr.get_current_size(), packet, offset);
			break;
		}
		case NBTType::TAG_LIST:
		{
			NBTList nbt_list = boost::any_cast<NBTList>(this->value);
			PacketEncoder::WriteByte((uint8_t)nbt_list.GetType(), packet, offset);
			PacketEncoder::WriteInt((uint32_t)nbt_list.GetElements().size(), packet, offset);
			for (NBTTag tag : nbt_list.GetElements())
			{
				tag.Serialize(packet, offset);
			}
			break;
		}
		case NBTType::TAG_COMPOUND:
		{
			NBTTagCompound nbt_compound = boost::any_cast<NBTTagCompound>(this->value);
			nbt_compound.Serialize(packet, offset);
			break;
		}
		}
	}

	std::string NBTTag::GetName()
	{
		return this->name;
	}

	NBTType NBTTag::GetType()
	{
		return this->type;
	}

	boost::any NBTTag::GetValue()
	{
		return this->value;
	}
}