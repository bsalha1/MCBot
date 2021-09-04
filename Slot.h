#pragma once

#include "Serializable.h"
#include "NBTTagCompound.h"

namespace McBot
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

		void Serialize(Packet& packet) const override;

		short GetID();
		uint8_t GetCount();
		short GetData();
		NBTTagCompound GetNBT();

		std::string ToString();
	};
}

