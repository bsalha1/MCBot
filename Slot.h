#pragma once

#include "NBT.h"

namespace mcbot
{
	class Slot : Serializable<Slot>
	{
	private:
		short item_id;
		uint8_t item_count;
		short data;
		mcbot::NBT nbt;

	public:
		Slot();

		Slot(short item_id, int item_count, short data, mcbot::NBT nbt);

		void serialize(Slot object, uint8_t* packet, size_t& offset) override;\

		std::string to_string();
	};
}

