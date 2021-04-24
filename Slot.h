#pragma once

#include "NBT.h"

namespace mcbot
{
	class Slot
	{
	private:
		bool item_present;
		int item_id;
		int item_count;
		mcbot::NBT nbt;
	public:
		Slot(bool item_present);

		Slot(bool item_present, int item_id, int item_count, mcbot::NBT nbt);
	};
}

