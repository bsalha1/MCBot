#include "MojangSession.h"

mcbot::MojangSession::MojangSession(std::string access_token, std::string username, std::string uuid)
{
	this->access_token = access_token;
	this->username = username;
	this->uuid = uuid;
}

mcbot::MojangSession::MojangSession()
{
}

std::string mcbot::MojangSession::get_access_token()
{
	return this->access_token;
}

std::string mcbot::MojangSession::get_username()
{
	return this->username;
}

std::string mcbot::MojangSession::get_uuid()
{
	return this->uuid;
}
