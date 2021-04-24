#include "UUID.h"

mcbot::UUID::UUID(int64_t msb, int64_t lsb)
{
	this->msb = msb;
	this->lsb = lsb;
}

mcbot::UUID::UUID()
{
	this->msb = 0;
	this->lsb = 0;
}

std::string mcbot::UUID::to_string()
{
	std::string str = "";

	for (int i = 0; i < 64; i += 8)
	{
		str += (lsb >> i) & 0xFF;
	}

	for (int i = 0; i < 64; i += 8)
	{
		str += (lsb >> i) & 0xFF;
	}

	return str;
}

std::ostream& mcbot::operator<<(std::ostream& os, UUID uuid)
{
	os << uuid.to_string();
	return os;
}

bool mcbot::operator<(UUID uuid1, UUID uuid2)
{
	if (uuid1.msb > uuid2.msb)
	{
		return true;
	}
	else if (uuid1.msb < uuid2.msb)
	{
		return false;
	}
	else if (uuid1.lsb > uuid2.lsb)
	{
		return true;
	}

	return false;
}
