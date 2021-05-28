#pragma once

#include "Serializable.h"
#include "NBTTagCompound.h"

namespace mcbot
{
	class Slot : Serializable
	{
	private:
		short id;
		uint8_t count;
		short data;
		NBTTagCompound nbt;

	public:
		Slot();

		Slot(short id, int count, short data, NBTTagCompound nbt);

		void Serialize(uint8_t* packet, size_t& offset) const override;

		short GetID();
		uint8_t GetCount();
		short GetData();
		NBTTagCompound GetNBT();

		std::string ToString();
	};
}

