#include "Packet.h"

namespace McBot
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
		this->data = NULL;
	}

	void Packet::AddByte(uint8_t byte)
	{
		data[offset++] = byte;
	}

	uint8_t Packet::PopByte()
	{
		return data[offset++];
	}

	uint8_t Packet::GetCurrentByte()
	{
		return data[offset];
	}
}
