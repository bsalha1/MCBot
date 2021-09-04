#include "UUID.h"

namespace McBot
{

	UUID::UUID(char big_endian_bytes[16])
	{
		for (int i = 0; i < 16; i++)
		{
			this->bytes[i] = big_endian_bytes[i];
		}

		this->msb =
			(uint64_t)this->bytes[15] << 56 |
			(uint64_t)this->bytes[14] << 48 |
			(uint64_t)this->bytes[13] << 40 |
			(uint64_t)this->bytes[12] << 32 |
			(uint64_t)this->bytes[11] << 24 |
			(uint64_t)this->bytes[10] << 16 |
			(uint64_t)this->bytes[9] << 8 |
			(uint64_t)this->bytes[8] << 0;

		this->lsb =
			(uint64_t)this->bytes[7] << 56 |
			(uint64_t)this->bytes[6] << 48 |
			(uint64_t)this->bytes[5] << 40 |
			(uint64_t)this->bytes[4] << 32 |
			(uint64_t)this->bytes[3] << 24 |
			(uint64_t)this->bytes[2] << 16 |
			(uint64_t)this->bytes[1] << 8 |
			(uint64_t)this->bytes[0] << 0;
	}

	UUID::UUID()
	{
		this->msb = 0;
		this->lsb = 0;
	}

	std::string UUID::ToString()
	{
		char str[37] = {};
		sprintf_s(str,
			"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			this->bytes[0], this->bytes[1], this->bytes[2], this->bytes[3], this->bytes[4], this->bytes[5], this->bytes[6], this->bytes[7],
			this->bytes[8], this->bytes[9], this->bytes[10], this->bytes[11], this->bytes[12], this->bytes[13], this->bytes[14], this->bytes[15]
		);

		return str;
	}

	/*std::ostream& operator<<(std::ostream& os, UUID uuid)
	{
		os << uuid.to_string();
		return os;
	}*/

	bool operator==(const UUID& lhs, const UUID& rhs)
	{
		return lhs.msb == rhs.msb && lhs.lsb == rhs.lsb;
	}

	bool operator<(const UUID& uuid1, const UUID& uuid2)
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
}
