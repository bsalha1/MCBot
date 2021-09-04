#pragma once

#include <set>

namespace McBot
{
	class BlockUtils
	{
	private:
		static std::set<int> pass_through_blocks;

	public:
		BlockUtils() = default;

		/*
			@param id: ID of block
			@return: true if the block can be passed through by the player
		*/
		static bool CanPassThrough(int id);
	};
}

