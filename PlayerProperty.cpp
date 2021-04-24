#include "PlayerProperty.h"

mcbot::PlayerProperty::PlayerProperty(std::string name, std::string value, bool _signed, std::string signature)
{
	this->name = name;
	this->value = value;
	this->_signed = _signed;

	if (_signed)
	{
		this->signature = signature;
	}
	else
	{
		this->signature = "";
	}
}

std::string mcbot::PlayerProperty::get_name()
{
	return this->name;
}

std::string mcbot::PlayerProperty::get_value()
{
	return this->value;
}

bool mcbot::PlayerProperty::is_signed()
{
	return this->_signed;
}

std::string mcbot::PlayerProperty::get_signature()
{
	return this->signature;
}
