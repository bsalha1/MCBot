#pragma once

#include "Slot.h"
#include "Color.h"
#include <boost/any.hpp>

namespace McBot
{
	class Particle
	{
	private:
		int id;
		boost::any data;

	public:
		Particle(int id);
		Particle(int id, McBot::Slot slot);
		Particle(int id, McBot::Color color);
		Particle(int id, int block_state);
	};
}

