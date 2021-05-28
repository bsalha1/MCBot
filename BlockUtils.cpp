#include "BlockUtils.h"

namespace mcbot
{

	std::set<int> BlockUtils::weak_blocks = {
		0, 6, 31, 32, 37, 38, 39, 40, 50, 51, 55, 59, 63, 69, 75, 76, 77, 83, 104, 105, 115, 132, 143, 175, 207,
	};

	BlockUtils::BlockUtils()
	{

	}

	bool BlockUtils::IsWeak(int id)
	{
		return BlockUtils::weak_blocks.find(id) != BlockUtils::weak_blocks.end();
	}
}
