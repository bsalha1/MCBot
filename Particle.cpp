#include "Particle.h"


mcbot::Particle::Particle(int id)
{
	this->id = id;
}

mcbot::Particle::Particle(int id, mcbot::Slot data)
{
	this->id = id;
	this->data = data;
}

mcbot::Particle::Particle(int id, mcbot::Color color)
{
	this->id = id;
	this->data = color;
}

mcbot::Particle::Particle(int id, int block_state)
{
	this->id = id;
	this->data = block_state;
}
