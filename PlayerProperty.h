#pragma once

#include <iostream>

namespace mcbot
{
	class PlayerProperty
	{
	private:
		std::string name;
		std::string value;
		bool _signed;
		std::string signature;
	public:
		PlayerProperty(std::string name, std::string value, bool _signed, std::string signature);

		std::string get_name();
		std::string get_value();
		bool is_signed();
		std::string get_signature();
	};
}

