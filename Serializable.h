#pragma once

#include <iostream>

namespace mcbot
{
	class Serializable
	{
	public:
		virtual void Serialize(uint8_t* packet, size_t& offset) const = 0;
	};
}

