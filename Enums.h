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

	enum class PlayerInfoAction {
		ADD_PLAYER = 0,
		UPDATE_GAMEMODE = 1,
		UPDATE_LATENCY = 2,
		UPDATE_DISPLAY_NAME = 3,
		REMOVE_PLAYER = 4
	};

	enum class ChatPosition {
		CHAT = 0,
		SYSTEM = 1,
		HOTBAR = 2
	};

	// Scoreboards //
	enum class FriendlyFire {
		OFF = 0,
		ON = 1,
		SEEING_INVISIBLE = 3
	};

	enum class ScoreboardMode {
		CREATE = 0,
		REMOVE = 1,
		UPDATE = 2,
		PLAYER_ADDED = 3,
		PLAYER_REMOVED = 4,
	};

	enum class ScoreboardPosition {
		LIST = 0,
		SIDEBAR = 1,
		BELOW_NAME = 2
	};

	enum class ScoreAction {
		UPDATE = 0,
		REMOVE = 1
	};
}

