#include "Attribute.h"

namespace McBot
{

	Attribute::Attribute(std::string key, double value, std::list<AttributeModifier> modifiers)
	{
		this->key = key;
		this->value = value;
		this->modifiers = modifiers;
	}
}