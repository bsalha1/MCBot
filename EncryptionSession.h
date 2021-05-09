#pragma once

#include <string>

namespace mcbot
{
	class EncryptionSession
	{
	private:
		std::string verify_token;
		std::string shared_secret;
		std::string public_key;
	};
}

