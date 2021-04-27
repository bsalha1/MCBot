#include "Attribute.h"

mcbot::Attribute::Attribute(std::string key, double value, std::list<mcbot::AttributeModifier> modifiers)
{
	this->key = key;
	this->value = value;
	this->modifiers = modifiers;
}
