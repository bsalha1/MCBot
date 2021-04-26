#pragma once

#include "Slot.h"
#include "Color.h"
#include <boost/any.hpp>

namespace mcbot
{
	class Particle
	{
	private:
		int id;
		boost::any data;

	public:
		Particle(int id);
		Particle(int id, mcbot::Slot slot);
		Particle(int id, mcbot::Color color);
		Particle(int id, int block_state);
	};
}

