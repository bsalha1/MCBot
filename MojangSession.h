#pragma once

#include <string>

namespace McBot
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

		std::string GetAccessToken();
		std::string GetUsername();
		std::string GetUUID();
	};
}

