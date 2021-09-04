#pragma once

#include <iostream>

#include "Enums.h"

namespace McBot
{
	class StringUtils
	{
	public:
		static std::string to_string(McBot::State state);

		static std::string to_string(McBot::Gamemode c);

		static std::string to_string(McBot::Dimension c);

		static std::string to_string(McBot::Difficulty c);

		static std::string to_string(McBot::NBTType c);

		static std::string to_string(McBot::TitleAction c);

		static std::string to_string(McBot::ParticleType c);

		static std::string to_string(McBot::EntityStatus c);

		static std::string to_string(McBot::WorldBorderAction c);

		static std::string to_string(McBot::PlayerInfoAction c);

		static std::string to_string(McBot::TileEntityAction c);

		static std::string to_string(McBot::ChatPosition c);

		static std::string to_string(McBot::FriendlyFire c);

		static std::string to_string(McBot::ScoreboardMode c);

		static std::string to_string(McBot::ScoreboardPosition c);

		static std::string to_string(McBot::ScoreAction c);
	};
}