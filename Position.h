#pragma once

#include <iostream>
#include "Vector.h"

namespace McBot
{
	class Position
	{
	private:
		int32_t x; // 26 bit integer
		int16_t y; // 12 bit integer
		int32_t z; // 26 bit integer

	public:
		Position(int32_t x, int32_t y, int32_t z);

		int32_t GetX();
		int16_t GetY();
		int32_t GetZ();

		Vector<int> ToVector();
		std::string ToString();
	};
}

