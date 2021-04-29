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

	enum class Direction {
		DOWN = 0,
		UP = 1,
		NORTH = 2,
		SOUTH = 3,
		WEST = 4,
		EAST = 5
	};

	enum class EntityStatus {
		RESET_MOB_SPAWN_MINECART = 1, // Also rabbit jump animation
		HURT = 2,
		DEAD = 3,
		IRON_GOLEM_THROW = 4,
		TAMING = 6,
		TAMED = 7,
		WOLF_SHAKING = 8,
		EATING = 9,
		SHEEP_EATING = 10, // Also TNT ignite sound
		IRON_GOLEM_ROSE = 11,
		VILLAGER_MATING = 12,
		VILLAGER_ANGRY = 13,
		VILLAGER_HAPPY = 14,
		WITCH_PARTICLES = 15,
		ZOMBIE_TO_VILLAGE_SOUND = 16,
		FIREWORK_EXPLODE = 17,
		ANIMAL_READY_TO_MATE = 18,
		SQUID_RESET_ROTATION = 19,
		EXPLOSION_PARTICLE = 20,
		GUARDIAN_SOUND = 21,
		ENABLE_DEBUG = 22,
		DISABLE_DEBUG = 23
	};

	enum class ParticleType {
		EXPLODE = 0,
		LARGE_EXPLOSION = 1,
		HUGE_EXPLOSION = 2,
		FIREWORKS_SPARK = 3,
		BUBBLE = 4,
		SPLASH = 5,
		WAKE = 6,
		SUSPENDED = 7,
		DEPTH_SUSPEND = 8,
		CRIT = 9,
		MAGIC_CRIT = 10,
		SMOKE = 11,
		LARGE_SMOKE = 12,
		SPELL = 13,
		INSTANT_SPELL = 14,
		MOB_SPELL = 15,
		MOB_SPELL_AMBIENT = 16,
		WITCH_MAGIC = 17,
		DRIP_WATER = 18,
		DRIP_LAVA = 19,
		ANGRY_VILLAGER = 20,
		HAPPY_VILLAGER = 21,
		TOWN_AURA = 22,
		NOTE = 23,
		PORTAL = 24,
		ENCHANTMENT_TABLE = 25,
		FLAME = 26,
		LAVA = 27,
		FOOTSTEP = 28,
		CLOUD = 29,
		RED_DUST = 30,
		SNOWBALL_POOF = 31,
		SNOW_SHOVEL = 32,
		SLIME = 33,
		HEART = 34,
		BARRIER = 35,
		ICON_CRACK = 36,
		BLOCK_CRACK = 37,
		BLOCK_DUST = 38,
		DROPLET = 39,
		TAKE = 40,
		MOB_APPEARANCE = 41,
	};
	
	enum class Pose {
		STANDING = 0,
		FALL_FLYING = 1,
		SLEEPING = 2,
		SWIMMING = 3,
		SPIN_ATTACK = 4,
		SNEAKING = 5,
		DYING = 6
	};

	enum class VillageProfession {

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

	enum class TitleAction {
		SET_TITLE = 0,
		SET_SUBTITLE = 1,
		SET_TIMES_AND_DISPLAY = 2,
		HIDE = 3,
		RESET = 4
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

	enum class TileEntityAction {
		SPAWN_POTENTIAL = 1, // set spawner potential
		COMMAND_BLOCK = 2,   // command block text change
		BEACON = 3,          // set level, primary and secondary powers of beacon
		SKULL = 4,           // set rotation and skin of skull
		FLOWER_POT = 5,      // set flower in pot
		BANNER = 6           // set base color and patterns of banner
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

