#pragma once

#include <stdint.h>

namespace McBot
{
	struct Packet
	{
		uint8_t* data;
		size_t length;
		size_t offset;

		Packet();
		Packet(size_t length);

		void AddByte(uint8_t byte);

		uint8_t PopByte();

		uint8_t GetCurrentByte();
	};
}

