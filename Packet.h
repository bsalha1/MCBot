#pragma once

#include <stdint.h>

namespace mcbot
{
	struct Packet
	{
	public:
		Packet();
		Packet(size_t length);

		uint8_t* data;
		size_t length;
		size_t offset;
	};
}

