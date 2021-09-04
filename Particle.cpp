#include "Particle.h"

namespace McBot
{

	Particle::Particle(int id)
	{
		this->id = id;
	}

	Particle::Particle(int id, Slot data)
	{
		this->id = id;
		this->data = data;
	}

	Particle::Particle(int id, Color color)
	{
		this->id = id;
		this->data = color;
	}

	Particle::Particle(int id, int block_state)
	{
		this->id = id;
		this->data = block_state;
	}
}
