#pragma once

#include <iostream>

#include "Vector.h"

namespace mcbot
{
	class Position
	{
	private:
		int32_t x; // 26 bit integer
		int16_t y; // 12 bit integer
		int32_t z; // 26 bit integer

	public:
		Position(int32_t x, int32_t y, int32_t z);

		int32_t get_x();
		int16_t get_y();
		int32_t get_z();

		mcbot::Vector<int> to_vector();

		std::string to_string();
	};
}

