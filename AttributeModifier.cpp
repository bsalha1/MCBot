#include "AttributeModifier.h"

namespace McBot
{

	AttributeModifier::AttributeModifier(UUID uuid, double amount, uint8_t operation)
	{
		this->uuid = uuid;
		this->amount = amount;
		this->operation = operation;
	}
}
