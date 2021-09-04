#pragma once

#include <list>
#include <string>

#include "AttributeModifier.h"

namespace McBot
{
	class Attribute
	{
	private:
		std::string key;
		double value;
		std::list<AttributeModifier> modifiers;
		
	public:
		Attribute(std::string key, double value, std::list<AttributeModifier> modifiers);
	};
}

