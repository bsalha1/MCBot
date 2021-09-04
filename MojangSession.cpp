#include "MojangSession.h"

namespace McBot
{
	MojangSession::MojangSession(std::string access_token, std::string username, std::string uuid)
	{
		this->access_token = access_token;
		this->username = username;
		this->uuid = uuid;
	}

	MojangSession::MojangSession()
	{
	}

	std::string MojangSession::GetAccessToken()
	{
		return this->access_token;
	}

	std::string MojangSession::GetUsername()
	{
		return this->username;
	}

	std::string MojangSession::GetUUID()
	{
		return this->uuid;
	}
}
