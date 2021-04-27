#pragma once

#include <list>
#include <string>

#include "AttributeModifier.h"

namespace mcbot
{
	class Attribute
	{
	private:
		std::string key;
		double value;
		std::list<mcbot::AttributeModifier> modifiers;
		
	public:
		Attribute(std::string key, double value, std::list<mcbot::AttributeModifier> modifiers);
	};
}

