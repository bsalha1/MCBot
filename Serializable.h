#pragma once

#include <iostream>

#include "Packet.h"

namespace mcbot
{
	class Serializable
	{
	public:
		virtual void Serialize(Packet& packet) const = 0;
	};
}

