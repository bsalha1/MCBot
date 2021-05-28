#pragma once

#include <iostream>

namespace mcbot
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

