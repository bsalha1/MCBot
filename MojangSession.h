#pragma once

#include <string>

namespace mcbot
{
	class MojangSession
	{
	private:
		std::string access_token;
		std::string username;
		std::string uuid;

	public:
		MojangSession(std::string access_token, std::string username, std::string uuid);
		MojangSession();

		std::string get_access_token();
		std::string get_username();
		std::string get_uuid();
	};
}

