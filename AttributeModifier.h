#pragma once

#include "UUID.h"

namespace mcbot
{
	class AttributeModifier
	{
	private:
		UUID uuid;
		double amount;
		uint8_t operation;

	public:
		AttributeModifier(UUID uuid, double amount, uint8_t operation);
	};
}

