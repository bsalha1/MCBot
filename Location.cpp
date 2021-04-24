#include <string>
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

std::string mcbot::Location::to_string()
{
	return "(" + std::to_string(this->x) + "," + std::to_string(this->y) + "," + std::to_string(this->z) + ")";
}
