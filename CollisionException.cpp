#include "CollisionException.h"

namespace mcbot
{

	CollisionException::CollisionException(Block block, Vector<double> location) noexcept
	{
		this->block = block;
		this->location = location;
		this->message = "CollisionException: Block " + std::to_string(this->block.GetID()) + " at destination location: " + this->location.ToString();
	}

	const char* CollisionException::what() const noexcept
	{
		return this->message.c_str();
	}
}
