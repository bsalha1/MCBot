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

		std::string GetName();
		std::string GetValue();
		bool IsSigned();
		std::string GetSignature();
	};
}

