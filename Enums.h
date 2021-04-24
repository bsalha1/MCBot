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

	enum class NBTType {
		TAG_END = 0,
		TAG_BYTE = 1,
		TAG_SHORT = 2,
		TAG_INT = 3,
		TAG_LONG = 4,
		TAG_FLOAT = 5,
		TAG_DOUBLE = 6,
		TAG_BYTE_ARRAY = 7,
		TAG_STRING = 8,
		TAG_LIST = 9,
		TAG_COMPOUND = 10,
		TAG_INT_ARRAY = 11,
		TAG_LONG_ARRAY = 12
	};

	enum class WorldBorderAction {
		SET_SIZE = 0,
		LERP_SIZE = 1,
		SET_CENTER = 2,
		INITIALIZE = 3,
		SET_WARNING_TIME = 4,
		SET_WARNING_BLOCKS = 5
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

