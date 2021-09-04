#pragma once

#include <iostream>

namespace McBot
{
	class Statistic
	{
	private:
		std::string name;
		int value;

	public:
		Statistic(std::string name, int value);

		std::string GetName();
		int GetValue();
		
		std::string ToString();
	};
}

