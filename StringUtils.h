#pragma once

#include <iostream>

#include "Enums.h"

namespace mcbot
{
	class StringUtils
	{
	public:
		static std::string to_string(mcbot::State state);

		static std::string to_string(mcbot::Gamemode c);

		static std::string to_string(mcbot::Dimension c);

		static std::string to_string(mcbot::Difficulty c);

		static std::string to_string(mcbot::NBTType c);

		static std::string to_string(mcbot::TitleAction c);

		static std::string to_string(mcbot::ParticleType c);

		static std::string to_string(mcbot::EntityStatus c);

		static std::string to_string(mcbot::WorldBorderAction c);

		static std::string to_string(mcbot::PlayerInfoAction c);

		static std::string to_string(mcbot::TileEntityAction c);

		static std::string to_string(mcbot::ChatPosition c);

		static std::string to_string(mcbot::FriendlyFire c);

		static std::string to_string(mcbot::ScoreboardMode c);

		static std::string to_string(mcbot::ScoreboardPosition c);

		static std::string to_string(mcbot::ScoreAction c);
	};
}