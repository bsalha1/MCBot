#include "Block.h"

namespace McBot
{

	Block::Block(int id)
	{
		this->id = id;
	}

	Block::Block()
	{
		this->id = 0;
	}

	bool Block::CanPassThrough()
	{
		return BlockUtils::CanPassThrough(this->id);
	}

	int Block::GetID()
	{
		return this->id;
	}
}
