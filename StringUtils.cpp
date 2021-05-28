#include "StringUtils.h"

namespace mcbot
{
	std::string StringUtils::to_string(State state)
	{
		switch (state)
		{
		case State::HANDSHAKE: return "HANDSHAKE";
		case State::STATUS: return "STATUS";
		case State::LOGIN: return "LOGIN";
		case State::PLAY: return "PLAY";
		default: return "UNKNOWN";
		}
	}

	std::string StringUtils::to_string(Gamemode c)
	{
		switch (c)
		{
		case Gamemode::SURVIVAL: return "SURVIVAL";
		case Gamemode::CREATIVE: return "CREATIVE";
		case Gamemode::ADVENTURE: return "ADVENTURE";
		case Gamemode::SPECTATOR: return "SPECTATOR";
		default: return "UNKNOWN";
		}
	}

	std::string StringUtils::to_string(Dimension c)
	{
		switch (c)
		{
		case Dimension::NETHER: return "NETHER";
		case Dimension::OVERWORLD: return "OVERWORLD";
		case Dimension::END: return "END";
		default: return "UNKNOWN";
		}
	}

	std::string StringUtils::to_string(Difficulty c)
	{
		switch (c)
		{
		case Difficulty::PEACEFUL: return "PEACEFUL";
		case Difficulty::EASY: return "EASY";
		case Difficulty::NORMAL: return "NORMAL";
		case Difficulty::HARD: return "HARD";
		default: return "UNKNOWN";
		}
	}

	std::string StringUtils::to_string(NBTType c)
	{
		switch (c)
		{
		case NBTType::TAG_END: return "TAG_END";
		case NBTType::TAG_BYTE: return "TAG_BYTE";
		case NBTType::TAG_SHORT: return "TAG_SHORT";
		case NBTType::TAG_INT: return "TAG_INT";
		case NBTType::TAG_LONG: return "TAG_LONG";
		case NBTType::TAG_FLOAT: return "TAG_FLOAT";
		case NBTType::TAG_DOUBLE: return "TAG_DOUBLE";
		case NBTType::TAG_BYTE_ARRAY: return "TAG_BYTE_ARRAY";
		case NBTType::TAG_STRING: return "TAG_STRING";
		case NBTType::TAG_LIST: return "TAG_LIST";
		case NBTType::TAG_COMPOUND: return "TAG_COMPOUND";
		case NBTType::TAG_INT_ARRAY: return "TAG_INT_ARRAY";
		case NBTType::TAG_LONG_ARRAY: return "TAG_LONG_ARRAY";
		default: return "UNKNOWN";
		}
	}

	std::string StringUtils::to_string(TitleAction c)
	{
		switch (c)
		{
		case TitleAction::SET_TITLE: return "SET_TITLE";
		case TitleAction::SET_SUBTITLE: return "SET_SUBTITLE";
		case TitleAction::SET_TIMES_AND_DISPLAY: return "SET_TIMES_AND_DISPLAY";
		case TitleAction::HIDE: return "HIDE";
		case TitleAction::RESET: return "RESET";
		default: return "UNKNOWN";
		}
	}

	std::string StringUtils::to_string(ParticleType c)
	{
		switch (c)
		{
		case ParticleType::ANGRY_VILLAGER: return "ANGRY_VILLAGER";
		case ParticleType::BARRIER: return "BARRIER";
		case ParticleType::BLOCK_CRACK: return "BLOCK_CRACK";
		case ParticleType::BLOCK_DUST: return "BLOCK_DUST";
		case ParticleType::BUBBLE: return "BUBBLE";
		case ParticleType::CLOUD: return "CLOUD";
		case ParticleType::CRIT: return "CRIT";
		case ParticleType::DEPTH_SUSPEND: return "DEPTH_SUSPEND";
		case ParticleType::DRIP_LAVA: return "DRIP_LAVA";
		case ParticleType::DRIP_WATER: return "DRIP_WATER";
		case ParticleType::DROPLET: return "DROPLET";
		case ParticleType::ENCHANTMENT_TABLE: return "ENCHANTMENT_TABLE";
		case ParticleType::EXPLODE: return "EXPLODE";
		case ParticleType::FIREWORKS_SPARK: return "FIREWORKS_SPARK";
		case ParticleType::FLAME: return "FLAME";
		case ParticleType::FOOTSTEP: return "FOOTSTEP";
		case ParticleType::HAPPY_VILLAGER: return "HAPPY_VILLAGER";
		case ParticleType::HEART: return "HEART";
		case ParticleType::HUGE_EXPLOSION: return "HUGE_EXPLOSION";
		case ParticleType::ICON_CRACK: return "ICON_CRACK";
		case ParticleType::INSTANT_SPELL: return "INSTANT_SPELL";
		case ParticleType::LARGE_EXPLOSION: return "LARGE_EXPLOSION";
		case ParticleType::LARGE_SMOKE: return "LARGE_SMOKE";
		case ParticleType::LAVA: return "LAVA";
		case ParticleType::MAGIC_CRIT: return "MAGIC_CRIT";
		case ParticleType::MOB_APPEARANCE: return "MOB_APPEARANCE";
		case ParticleType::MOB_SPELL: return "MOB_SPELL";
		case ParticleType::MOB_SPELL_AMBIENT: return "MOB_SPELL_AMBIENT";
		case ParticleType::NOTE: return "NOTE";
		case ParticleType::PORTAL: return "PORTAL";
		case ParticleType::RED_DUST: return "RED_DUST";
		case ParticleType::SLIME: return "SLIME";
		case ParticleType::SMOKE: return "SMOKE";
		case ParticleType::SNOWBALL_POOF: return "SNOWBALL_POOF";
		case ParticleType::SNOW_SHOVEL: return "SNOW_SHOVEL";
		case ParticleType::SPELL: return "SPELL";
		case ParticleType::SPLASH: return "SPLASH";
		case ParticleType::SUSPENDED: return "SUSPENDED";
		case ParticleType::TAKE: return "TAKE";
		case ParticleType::TOWN_AURA: return "TOWN_AURA";
		case ParticleType::WAKE: return "WAKE";
		case ParticleType::WITCH_MAGIC: return "WITCH_MAGIC";
		default: return "UNKNOWN";
		}
	}

	std::string StringUtils::to_string(EntityStatus c)
	{
		switch (c)
		{
		case EntityStatus::ANIMAL_READY_TO_MATE: return "ANIMAL_READY_TO_MATE";
		case EntityStatus::DEAD: return "DEAD";
		case EntityStatus::DISABLE_DEBUG: return "DISABLE_DEBUG";
		case EntityStatus::EATING: return "EATING";
		case EntityStatus::ENABLE_DEBUG: return "ENABLE_DEBUG";
		case EntityStatus::EXPLOSION_PARTICLE: return "EXPLOSION_PARTICLE";
		case EntityStatus::FIREWORK_EXPLODE: return "FIREWORK_EXPLODE";
		case EntityStatus::GUARDIAN_SOUND: return "GUARDIAN_SOUND";
		case EntityStatus::HURT: return "HURT";
		case EntityStatus::IRON_GOLEM_ROSE: return "IRON_GOLEM_ROSE";
		case EntityStatus::IRON_GOLEM_THROW: return "IRON_GOLEM_THROW";
		case EntityStatus::RESET_MOB_SPAWN_MINECART: return "RESET_MOB_SPAWN_MINECART";
		case EntityStatus::SHEEP_EATING: return "SHEEP_EATING";
		case EntityStatus::SQUID_RESET_ROTATION: return "SQUID_RESET_ROTATION";
		case EntityStatus::TAMED: return "TAMED";
		case EntityStatus::TAMING: return "TAMING";
		case EntityStatus::VILLAGER_ANGRY: return "VILLAGER_ANGRY";
		case EntityStatus::VILLAGER_HAPPY: return "VILLAGER_HAPPY";
		case EntityStatus::VILLAGER_MATING: return "VILLAGER_MATING";
		case EntityStatus::WITCH_PARTICLES: return "WITCH_PARTICLES";
		case EntityStatus::WOLF_SHAKING: return "WOLF_SHAKING";
		case EntityStatus::ZOMBIE_TO_VILLAGE_SOUND: return "ZOMBIE_TO_VILLAGE_SOUND";
		default: return "UNKNOWN";
		}
	}

	std::string StringUtils::to_string(WorldBorderAction c)
	{
		switch (c)
		{
		case WorldBorderAction::SET_SIZE: return "SET_SIZE";
		case WorldBorderAction::LERP_SIZE: return "LERP_SIZE";
		case WorldBorderAction::SET_CENTER: return "SET_CENTER";
		case WorldBorderAction::INITIALIZE: return "INITIALIZE";
		case WorldBorderAction::SET_WARNING_TIME: return "SET_WARNING_TIME";
		case WorldBorderAction::SET_WARNING_BLOCKS: return "SET_WARNING_BLOCKS";
		default: return "UNKNOWN";
		}
	}

	std::string StringUtils::to_string(PlayerInfoAction c)
	{
		switch (c)
		{
		case PlayerInfoAction::ADD_PLAYER: return "ADD_PLAYER";
		case PlayerInfoAction::UPDATE_GAMEMODE: return "UPDATE_GAMEMODE";
		case PlayerInfoAction::UPDATE_LATENCY: return "UPDATE_LATENCY";
		case PlayerInfoAction::UPDATE_DISPLAY_NAME: return "UPDATE_DISPLAY_NAME";
		case PlayerInfoAction::REMOVE_PLAYER: return "REMOVE_PLAYER";
		default: return "UNKNOWN";
		}
	}

	std::string StringUtils::to_string(TileEntityAction c)
	{
		switch (c)
		{
		case TileEntityAction::BANNER: return "BANNER";
		case TileEntityAction::BEACON: return "BEACON";
		case TileEntityAction::COMMAND_BLOCK: return "COMMAND_BLOCK";
		case TileEntityAction::FLOWER_POT: return "FLOWER_POT";
		case TileEntityAction::SKULL: return "SKULL";
		case TileEntityAction::SPAWN_POTENTIAL: return "SPAWN_POTENTIAL";
		default: return "UNKNOWN";
		}
	}

	std::string StringUtils::to_string(ChatPosition c)
	{
		switch (c)
		{
		case ChatPosition::CHAT: return "CHAT";
		case ChatPosition::SYSTEM: return "SYSTEM";
		case ChatPosition::HOTBAR: return "HOTBAR";
		default: return "UNKNOWN";
		}
	}

	std::string StringUtils::to_string(FriendlyFire c)
	{
		switch (c)
		{
		case FriendlyFire::OFF: return "OFF";
		case FriendlyFire::ON: return "ON";
		case FriendlyFire::SEEING_INVISIBLE: return "SEEING_INVISIBLE";
		default: return "UNKNOWN";
		}
	}

	std::string StringUtils::to_string(ScoreboardMode c)
	{
		switch (c)
		{
		case ScoreboardMode::CREATE: return "CREATE";
		case ScoreboardMode::REMOVE: return "REMOVE";
		case ScoreboardMode::UPDATE: return "UPDATE";
		case ScoreboardMode::PLAYER_ADDED: return "PLAYER_ADDED";
		case ScoreboardMode::PLAYER_REMOVED: return "PLAYER_REMOVED";
		default: return "UNKNOWN";
		}
	}

	std::string StringUtils::to_string(ScoreboardPosition c)
	{
		switch (c)
		{
		case ScoreboardPosition::LIST: return "LIST";
		case ScoreboardPosition::SIDEBAR: return "SIDEBAR";
		case ScoreboardPosition::BELOW_NAME: return "BELOW_NAME";
		default: return "UNKNOWN";
		}
	}

	std::string StringUtils::to_string(ScoreAction c)
	{
		switch (c)
		{
		case ScoreAction::UPDATE: return "UPDATE";
		case ScoreAction::REMOVE: return "REMOVE";
		default: return "UNKNOWN";
		}
	}
}