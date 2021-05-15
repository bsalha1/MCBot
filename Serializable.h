#pragma once

#include <iostream>

namespace mcbot
{
	class Serializable
	{
		virtual void serialize(uint8_t* packet, size_t& offset) = 0;
	};
}

