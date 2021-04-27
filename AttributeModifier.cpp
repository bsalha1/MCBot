#include "AttributeModifier.h"

mcbot::AttributeModifier::AttributeModifier(mcbot::UUID uuid, double amount, uint8_t operation)
{
	this->uuid = uuid;
	this->amount = amount;
	this->operation = operation;
}
