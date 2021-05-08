#pragma once

#include <set>

namespace mcbot
{
	class BlockUtils
	{
	private:
		static std::set<int> weak_blocks; // These blocks don't change your Y-level when walked on

	public:
		BlockUtils();

		static bool is_weak(int id);
	};
}

