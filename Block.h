#pragma once

#include "BlockUtils.h"

namespace McBot
{
	class Block
	{
	private:
		int id;

	public:
		Block(int id);
		Block();

		bool CanPassThrough();

		int GetID();
	};
}

