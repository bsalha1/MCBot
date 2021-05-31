#pragma once

#include <set>

namespace mcbot
{
	class BlockUtils
	{
	private:
		static std::set<int> weak_blocks; 

	public:
		BlockUtils();

		/*
			@param id: ID of block
			@return: true if the block can be passed through by the player
		*/
		static bool IsWeak(int id);
	};
}

