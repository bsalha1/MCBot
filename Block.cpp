#include "Block.h"

namespace mcbot
{

	Block::Block(int id)
	{
		this->id = id;
	}

	Block::Block()
	{
		this->id = 0;
	}

	bool Block::IsWeak()
	{
		return BlockUtils::IsWeak(this->id);
	}

	int Block::GetID()
	{
		return this->id;
	}
}
