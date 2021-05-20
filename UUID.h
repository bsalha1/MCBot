#pragma once

#include <iostream>

namespace mcbot
{
	class UUID
	{
	private:
		uint8_t bytes[16] = { 0 };
		uint64_t msb;
		uint64_t lsb;

	public:
		UUID(char bytes[16]);

		UUID();

		std::string to_string();

		//friend std::ostream& operator<<(std::ostream& os, UUID uuid);

		friend bool operator<(const UUID & lhs, const UUID & rhs);
		friend bool operator==(const UUID & lhs, const UUID & rhs);
	};
}

