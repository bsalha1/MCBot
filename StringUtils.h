#pragma once

#include <iostream>

#include "Enums.h"
namespace mcbot
{
	inline std::string to_string(bool value)
	{
		return value ? "true" : "false";
	}

	std::string to_string(mcbot::State state)
	{
		switch (state)
		{
		case mcbot::State::HANDSHAKE: return "HANDSHAKE";
		case mcbot::State::STATUS: return "STATUS";
		case mcbot::State::LOGIN: return "LOGIN";
		case mcbot::State::PLAY: return "PLAY";
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::Gamemode c)
	{
		switch (c)
		{
		case mcbot::Gamemode::SURVIVAL: return "SURVIVAL";
		case mcbot::Gamemode::CREATIVE: return "CREATIVE";
		case mcbot::Gamemode::ADVENTURE: return "ADVENTURE";
		case mcbot::Gamemode::SPECTATOR: return "SPECTATOR";
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::Dimension c)
	{
		switch (c)
		{
		case mcbot::Dimension::NETHER: return "NETHER";
		case mcbot::Dimension::OVERWORLD: return "OVERWORLD";
		case mcbot::Dimension::END: return "END";
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::Difficulty c)
	{
		switch (c)
		{
		case mcbot::Difficulty::PEACEFUL: return "PEACEFUL";
		case mcbot::Difficulty::EASY: return "EASY";
		case mcbot::Difficulty::NORMAL: return "NORMAL";
		case mcbot::Difficulty::HARD: return "HARD";
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::NBTType c)
	{
		switch (c)
		{
		case mcbot::NBTType::TAG_END: return "TAG_END";
		case mcbot::NBTType::TAG_BYTE: return "TAG_BYTE";
		case mcbot::NBTType::TAG_SHORT: return "TAG_SHORT";
		case mcbot::NBTType::TAG_INT: return "TAG_INT";
		case mcbot::NBTType::TAG_LONG: return "TAG_LONG";
		case mcbot::NBTType::TAG_FLOAT: return "TAG_FLOAT";
		case mcbot::NBTType::TAG_DOUBLE: return "TAG_DOUBLE";
		case mcbot::NBTType::TAG_BYTE_ARRAY: return "TAG_BYTE_ARRAY";
		case mcbot::NBTType::TAG_STRING: return "TAG_STRING";
		case mcbot::NBTType::TAG_LIST: return "TAG_LIST";
		case mcbot::NBTType::TAG_COMPOUND: return "TAG_COMPOUND";
		case mcbot::NBTType::TAG_INT_ARRAY: return "TAG_INT_ARRAY";
		case mcbot::NBTType::TAG_LONG_ARRAY: return "TAG_LONG_ARRAY";
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::EntityStatus c)
	{
		switch (c)
		{
		case mcbot::EntityStatus::ANIMAL_READY_TO_MATE: return "ANIMAL_READY_TO_MATE";
		case mcbot::EntityStatus::DEAD: return "DEAD";
		case mcbot::EntityStatus::DISABLE_DEBUG: return "DISABLE_DEBUG";
		case mcbot::EntityStatus::EATING: return "EATING";
		case mcbot::EntityStatus::ENABLE_DEBUG: return "ENABLE_DEBUG";
		case mcbot::EntityStatus::EXPLOSION_PARTICLE: return "EXPLOSION_PARTICLE";
		case mcbot::EntityStatus::FIREWORK_EXPLODE: return "FIREWORK_EXPLODE";
		case mcbot::EntityStatus::GUARDIAN_SOUND: return "GUARDIAN_SOUND";
		case mcbot::EntityStatus::HURT: return "HURT";
		case mcbot::EntityStatus::IRON_GOLEM_ROSE: return "IRON_GOLEM_ROSE";
		case mcbot::EntityStatus::IRON_GOLEM_THROW: return "IRON_GOLEM_THROW";
		case mcbot::EntityStatus::RESET_MOB_SPAWN_MINECART: return "RESET_MOB_SPAWN_MINECART";
		case mcbot::EntityStatus::SHEEP_EATING: return "SHEEP_EATING";
		case mcbot::EntityStatus::SQUID_RESET_ROTATION: return "SQUID_RESET_ROTATION";
		case mcbot::EntityStatus::TAMED: return "TAMED";
		case mcbot::EntityStatus::TAMING: return "TAMING";
		case mcbot::EntityStatus::VILLAGER_ANGRY: return "VILLAGER_ANGRY";
		case mcbot::EntityStatus::VILLAGER_HAPPY: return "VILLAGER_HAPPY";
		case mcbot::EntityStatus::VILLAGER_MATING: return "VILLAGER_MATING";
		case mcbot::EntityStatus::WITCH_PARTICLES: return "WITCH_PARTICLES";
		case mcbot::EntityStatus::WOLF_SHAKING: return "WOLF_SHAKING";
		case mcbot::EntityStatus::ZOMBIE_TO_VILLAGE_SOUND: return "ZOMBIE_TO_VILLAGE_SOUND";
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::WorldBorderAction c)
	{
		switch (c)
		{
		case mcbot::WorldBorderAction::SET_SIZE: return "SET_SIZE";
		case mcbot::WorldBorderAction::LERP_SIZE: return "LERP_SIZE";
		case mcbot::WorldBorderAction::SET_CENTER: return "SET_CENTER";
		case mcbot::WorldBorderAction::INITIALIZE: return "INITIALIZE";
		case mcbot::WorldBorderAction::SET_WARNING_TIME: return "SET_WARNING_TIME";
		case mcbot::WorldBorderAction::SET_WARNING_BLOCKS: return "SET_WARNING_BLOCKS";
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::PlayerInfoAction c)
	{
		switch (c)
		{
		case mcbot::PlayerInfoAction::ADD_PLAYER: return "ADD_PLAYER";
		case mcbot::PlayerInfoAction::UPDATE_GAMEMODE: return "UPDATE_GAMEMODE";
		case mcbot::PlayerInfoAction::UPDATE_LATENCY: return "UPDATE_LATENCY";
		case mcbot::PlayerInfoAction::UPDATE_DISPLAY_NAME: return "UPDATE_DISPLAY_NAME";
		case mcbot::PlayerInfoAction::REMOVE_PLAYER: return "REMOVE_PLAYER";
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::TileEntityAction c)
	{
		switch (c)
		{
		case mcbot::TileEntityAction::BANNER: return "BANNER";
		case mcbot::TileEntityAction::BEACON: return "BEACON";
		case mcbot::TileEntityAction::COMMAND_BLOCK: return "COMMAND_BLOCK";
		case mcbot::TileEntityAction::FLOWER_POT: return "FLOWER_POT";
		case mcbot::TileEntityAction::SKULL: return "SKULL";
		case mcbot::TileEntityAction::SPAWN_POTENTIAL: return "SPAWN_POTENTIAL";
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::ChatPosition c)
	{
		switch (c)
		{
		case mcbot::ChatPosition::CHAT: return "CHAT";
		case mcbot::ChatPosition::SYSTEM: return "SYSTEM";
		case mcbot::ChatPosition::HOTBAR: return "HOTBAR";
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::FriendlyFire c)
	{
		switch (c)
		{
		case mcbot::FriendlyFire::OFF: return "OFF";
		case mcbot::FriendlyFire::ON: return "ON";
		case mcbot::FriendlyFire::SEEING_INVISIBLE: return "SEEING_INVISIBLE";
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::ScoreboardMode c)
	{
		switch (c)
		{
		case mcbot::ScoreboardMode::CREATE: return "CREATE";
		case mcbot::ScoreboardMode::REMOVE: return "REMOVE";
		case mcbot::ScoreboardMode::UPDATE: return "UPDATE";
		case mcbot::ScoreboardMode::PLAYER_ADDED: return "PLAYER_ADDED";
		case mcbot::ScoreboardMode::PLAYER_REMOVED: return "PLAYER_REMOVED";
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::ScoreboardPosition c)
	{
		switch (c)
		{
		case mcbot::ScoreboardPosition::LIST: return "LIST";
		case mcbot::ScoreboardPosition::SIDEBAR: return "SIDEBAR";
		case mcbot::ScoreboardPosition::BELOW_NAME: return "BELOW_NAME";
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::ScoreAction c)
	{
		switch (c)
		{
		case mcbot::ScoreAction::UPDATE: return "UPDATE";
		case mcbot::ScoreAction::REMOVE: return "REMOVE";
		default: return "UNKNOWN";
		}
	}
}