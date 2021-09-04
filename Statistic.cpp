#include <string>
#include "Statistic.h"

namespace McBot
{

	Statistic::Statistic(std::string name, int value)
	{
		this->name = name;
		this->value = value;
	}

	std::string Statistic::GetName()
	{
		return this->name;
	}

	int Statistic::GetValue()
	{
		return this->value;
	}

	std::string Statistic::ToString()
	{
		return "{" + this->name + "," + std::to_string(this->value) + "}";
	}
}
