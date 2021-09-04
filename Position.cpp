#include <string>
#include "Position.h"

namespace McBot
{

	Position::Position(int32_t x, int32_t y, int32_t z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	int32_t Position::GetX()
	{
		return this->x;
	}

	int16_t Position::GetY()
	{
		return this->y;
	}

	int32_t Position::GetZ()
	{
		return this->z;
	}

	Vector<int> Position::ToVector()
	{
		return Vector<int>(this->x, this->y, this->z);
	}

	std::string Position::ToString()
	{
		return "(" + std::to_string(this->x) + "," + std::to_string(this->y) + "," + std::to_string(this->z) + ")";
	}
}