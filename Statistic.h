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

		std::string get_name();
		int get_value();
		
		friend std::ostream& operator<<(std::ostream& os, mcbot::Statistic loc);
	};
}

