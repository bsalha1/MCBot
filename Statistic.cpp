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

std::ostream& mcbot::operator<<(std::ostream& os, mcbot::Statistic statistic)
{
	os << "{" << statistic.name << "," << statistic.value << "}";
	return os;
}
