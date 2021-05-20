#pragma once

#include "Serializable.h"
#include "NBTTagCompound.h"

namespace mcbot
{
	class Slot : Serializable
	{
	private:
		short item_id;
		uint8_t item_count;
		short data;
		mcbot::NBTTagCompound nbt;

	public:
		Slot();

		Slot(short item_id, int item_count, short data, mcbot::NBTTagCompound nbt);

		void serialize(uint8_t* packet, size_t& offset) override;

		std::string to_string();
	};
}

