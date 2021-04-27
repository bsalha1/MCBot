#pragma once

#include "UUID.h"

namespace mcbot
{
	class AttributeModifier
	{
	private:
		mcbot::UUID uuid;
		double amount;
		uint8_t operation;

	public:
		AttributeModifier(mcbot::UUID uuid, double amount, uint8_t operation);
	};
}

