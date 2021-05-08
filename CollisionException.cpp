#include "CollisionException.h"


mcbot::CollisionException::CollisionException(mcbot::Block block, mcbot::Vector<double> location) noexcept
{
	this->block = block;
	this->location = location;
	this->message = "CollisionException: Block " + std::to_string(this->block.get_id()) + " at destination location: " + this->location.to_string();
}

const char* mcbot::CollisionException::what() const noexcept
{
	return this->message.c_str();
}
