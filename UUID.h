#pragma once

#include <iostream>

namespace mcbot
{
	class UUID
	{
	private:
		int64_t msb;
		int64_t lsb;
	public:
		UUID(int64_t msb, int64_t lsb);

		UUID();

		std::string to_string();

		friend std::ostream& operator<<(std::ostream& os, UUID uuid);

		friend bool operator<(UUID uuid1, UUID uuid2);
	};
}

