#include "Slot.h"
#include "PacketEncoder.h"

namespace mcbot
{

	Slot::Slot()
	{
		this->id = -1;
		this->count = -1;
		this->data = -1;
	}

	Slot::Slot(short id, int count, short data, NBTTagCompound nbt)
	{
		this->id = id;
		this->count = count;
		this->data = data;
		this->nbt = nbt;
	}

	void Slot::Serialize(uint8_t* packet, size_t& offset) const
	{
		PacketEncoder::WriteShort(this->id, packet, offset);
		PacketEncoder::WriteByte(this->count, packet, offset);
		PacketEncoder::WriteShort(this->data, packet, offset);
		this->nbt.Serialize(packet, offset);
	}

	short Slot::GetID()
	{
		return this->id;
	}

	uint8_t Slot::GetCount()
	{
		return this->count;
	}

	short Slot::GetData()
	{
		return this->data;
	}

	NBTTagCompound Slot::GetNBT()
	{
		return this->nbt;
	}

	std::string Slot::ToString()
	{
		return "[id:" + std::to_string(id) +
			", count:" + std::to_string(count) +
			", data:" + std::to_string(data) + ", nbt:" + this->nbt.ToString() + "]";
	}
}
