#include "PlayerProperty.h"

namespace mcbot
{

	PlayerProperty::PlayerProperty(std::string name, std::string value, bool _signed, std::string signature)
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

	std::string PlayerProperty::GetName()
	{
		return this->name;
	}

	std::string PlayerProperty::GetValue()
	{
		return this->value;
	}

	bool PlayerProperty::IsSigned()
	{
		return this->_signed;
	}

	std::string PlayerProperty::GetSignature()
	{
		return this->signature;
	}
}
