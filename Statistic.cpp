#include <string>

#include "Statistic.h"

mcbot::Statistic::Statistic(std::string name, int value)
{
	this->name = name;
	this->value = value;
}

std::string mcbot::Statistic::get_name()
{
	return this->name;
}

int mcbot::Statistic::get_value()
{
	return this->value;
}

std::string mcbot::Statistic::to_string()
{
	return "{" + this->name + "," + std::to_string(this->value) + "}";
}
