#include "Slot.h"

mcbot::Slot::Slot(bool item_present)
{
	this->item_present = item_present;
	this->item_id = 0;
	this->item_count = 0;
}

mcbot::Slot::Slot(bool item_present, int item_id, int item_count, mcbot::NBT nbt)
{
	this->item_present = item_present;
	this->item_id = item_id;
	this->item_count = item_count;
	this->nbt = nbt;
}
