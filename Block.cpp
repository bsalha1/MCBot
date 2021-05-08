#include "Block.h"

mcbot::Block::Block(int id)
{
	this->id = id;
}

mcbot::Block::Block()
{
	this->id = 0;
}

bool mcbot::Block::is_weak()
{
	return BlockUtils::is_weak(this->id);
}

int mcbot::Block::get_id()
{
	return this->id;
}
