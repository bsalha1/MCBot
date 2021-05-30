#include "Packet.h"

namespace mcbot
{
	Packet::Packet()
	{
		this->length = 0;
		this->offset = 0;
		this->data = NULL;
	}

	Packet::Packet(size_t length)
	{
		this->length = length;
		this->offset = 0;
		this->data = new uint8_t[length];
	}
}
