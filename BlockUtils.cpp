#include "BlockUtils.h"

namespace mcbot
{
	std::set<int> BlockUtils::pass_through_blocks = {
		0, 6, 31, 32, 37, 38, 39, 40, 50, 51, 55, 59, 63, 69, 75, 76, 77, 83, 104, 105, 115, 132, 143, 175, 207,
	};

	bool BlockUtils::CanPassThrough(int id)
	{
		//return true;
		return BlockUtils::pass_through_blocks.find(id) != BlockUtils::pass_through_blocks.end();
	}
}
