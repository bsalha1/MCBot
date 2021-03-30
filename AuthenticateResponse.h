#pragma once

#include <iostream>
#include "Profile.h"
#include "User.h"

namespace mcbot
{
	class AuthenticateResponse
	{
	private:
		mcbot::User user;
		std::string clientToken;
		std::string accessToken;
		mcbot::Profile selectedProfile;
		mcbot::Profile availableProfiles[];
	};
}

