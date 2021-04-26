#include "Slot.h"

mcbot::Slot::Slot()
{
	this->item_id = -1;
	this->item_count = -1;
	this->data = -1;
}

mcbot::Slot::Slot(short item_id, int item_count, short data, mcbot::NBT nbt)
{
	this->item_id = item_id;
	this->item_count = item_count;
	this->data = data;
	this->nbt = nbt;
}