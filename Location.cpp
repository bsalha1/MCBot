#include "Location.h"

mcbot::Location::Location(int32_t x, int32_t y, int32_t z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

int32_t mcbot::Location::get_x()
{
	return this->x;
}

int16_t mcbot::Location::get_y()
{
	return this->y;
}

int32_t mcbot::Location::get_z()
{
	return this->z;
}

std::ostream& mcbot::operator<<(std::ostream& os, mcbot::Location loc)
{
	os << "(" << loc.x << "," << loc.y << "," << loc.z << ")";
	return os;
}
