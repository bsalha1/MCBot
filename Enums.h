#pragma once

#include <iostream>

namespace mcbot
{
	enum class State {
		HANDSHAKE,
		STATUS,
		LOGIN,
		PLAY
	};

	enum class Gamemode {
		SURVIVAL = 0,
		CREATIVE = 1,
		ADVENTURE = 2,
		SPECTATOR = 3
	};

	enum class Dimension {
		NETHER = -1,
		OVERWORLD = 0,
		END = 1
	};

	enum class Difficulty {
		PEACEFUL = 0,
		EASY = 1,
		NORMAL = 2,
		HARD = 3
	};

	enum class FriendlyFire {
		OFF = 0,
		ON = 1,
		SEEING_INVISIBLE = 3
	};
}


std::ostream& operator<<(std::ostream& os, mcbot::State c);

std::ostream& operator<<(std::ostream& os, mcbot::Gamemode c);

std::ostream& operator<<(std::ostream& os, mcbot::Dimension c);

std::ostream& operator<<(std::ostream& os, mcbot::Difficulty c);

std::ostream& operator<<(std::ostream& os, mcbot::FriendlyFire c);

