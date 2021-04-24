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
		case mcbot::State::HANDSHAKE: return "HANDSHAKE"; break;
		case mcbot::State::STATUS: return "STATUS"; break;
		case mcbot::State::LOGIN: return "LOGIN"; break;
		case mcbot::State::PLAY: return "PLAY"; break;
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::Gamemode c)
	{
		switch (c)
		{
		case mcbot::Gamemode::SURVIVAL: return "SURVIVAL"; break;
		case mcbot::Gamemode::CREATIVE: return "CREATIVE"; break;
		case mcbot::Gamemode::ADVENTURE: return "ADVENTURE"; break;
		case mcbot::Gamemode::SPECTATOR: return "SPECTATOR"; break;
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::Dimension c)
	{
		switch (c)
		{
		case mcbot::Dimension::NETHER: return "NETHER"; break;
		case mcbot::Dimension::OVERWORLD: return "OVERWORLD"; break;
		case mcbot::Dimension::END: return "END"; break;
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::Difficulty c)
	{
		switch (c)
		{
		case mcbot::Difficulty::PEACEFUL: return "PEACEFUL"; break;
		case mcbot::Difficulty::EASY: return "EASY"; break;
		case mcbot::Difficulty::NORMAL: return "NORMAL"; break;
		case mcbot::Difficulty::HARD: return "HARD"; break;
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::NBTType c)
	{
		switch (c)
		{
		case mcbot::NBTType::TAG_END: return "TAG_END"; break;
		case mcbot::NBTType::TAG_BYTE: return "TAG_BYTE"; break;
		case mcbot::NBTType::TAG_SHORT: return "TAG_SHORT"; break;
		case mcbot::NBTType::TAG_INT: return "TAG_INT"; break;
		case mcbot::NBTType::TAG_LONG: return "TAG_LONG"; break;
		case mcbot::NBTType::TAG_FLOAT: return "TAG_FLOAT"; break;
		case mcbot::NBTType::TAG_DOUBLE: return "TAG_DOUBLE"; break;
		case mcbot::NBTType::TAG_BYTE_ARRAY: return "TAG_BYTE_ARRAY"; break;
		case mcbot::NBTType::TAG_STRING: return "TAG_STRING"; break;
		case mcbot::NBTType::TAG_LIST: return "TAG_LIST"; break;
		case mcbot::NBTType::TAG_COMPOUND: return "TAG_COMPOUND"; break;
		case mcbot::NBTType::TAG_INT_ARRAY: return "TAG_INT_ARRAY"; break;
		case mcbot::NBTType::TAG_LONG_ARRAY: return "TAG_LONG_ARRAY"; break;
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::WorldBorderAction c)
	{
		switch (c)
		{
		case mcbot::WorldBorderAction::SET_SIZE: return "SET_SIZE"; break;
		case mcbot::WorldBorderAction::LERP_SIZE: return "LERP_SIZE"; break;
		case mcbot::WorldBorderAction::SET_CENTER: return "SET_CENTER"; break;
		case mcbot::WorldBorderAction::INITIALIZE: return "INITIALIZE"; break;
		case mcbot::WorldBorderAction::SET_WARNING_TIME: return "SET_WARNING_TIME"; break;
		case mcbot::WorldBorderAction::SET_WARNING_BLOCKS: return "SET_WARNING_BLOCKS"; break;
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::PlayerInfoAction c)
	{
		switch (c)
		{
		case mcbot::PlayerInfoAction::ADD_PLAYER: return "ADD_PLAYER"; break;
		case mcbot::PlayerInfoAction::UPDATE_GAMEMODE: return "UPDATE_GAMEMODE"; break;
		case mcbot::PlayerInfoAction::UPDATE_LATENCY: return "UPDATE_LATENCY"; break;
		case mcbot::PlayerInfoAction::UPDATE_DISPLAY_NAME: return "UPDATE_DISPLAY_NAME"; break;
		case mcbot::PlayerInfoAction::REMOVE_PLAYER: return "REMOVE_PLAYER"; break;
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::ChatPosition c)
	{
		switch (c)
		{
		case mcbot::ChatPosition::CHAT: return "CHAT"; break;
		case mcbot::ChatPosition::SYSTEM: return "SYSTEM"; break;
		case mcbot::ChatPosition::HOTBAR: return "HOTBAR"; break;
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::FriendlyFire c)
	{
		switch (c)
		{
		case mcbot::FriendlyFire::OFF: return "OFF"; break;
		case mcbot::FriendlyFire::ON: return "ON"; break;
		case mcbot::FriendlyFire::SEEING_INVISIBLE: return "SEEING_INVISIBLE"; break;
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::ScoreboardMode c)
	{
		switch (c)
		{
		case mcbot::ScoreboardMode::CREATE: return "CREATE"; break;
		case mcbot::ScoreboardMode::REMOVE: return "REMOVE"; break;
		case mcbot::ScoreboardMode::UPDATE: return "UPDATE"; break;
		case mcbot::ScoreboardMode::PLAYER_ADDED: return "PLAYER_ADDED"; break;
		case mcbot::ScoreboardMode::PLAYER_REMOVED: return "PLAYER_REMOVED"; break;
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::ScoreboardPosition c)
	{
		switch (c)
		{
		case mcbot::ScoreboardPosition::LIST: return "LIST"; break;
		case mcbot::ScoreboardPosition::SIDEBAR: return "SIDEBAR"; break;
		case mcbot::ScoreboardPosition::BELOW_NAME: return "BELOW_NAME"; break;
		default: return "UNKNOWN";
		}
	}

	std::string to_string(mcbot::ScoreAction c)
	{
		switch (c)
		{
		case mcbot::ScoreAction::UPDATE: return "UPDATE"; break;
		case mcbot::ScoreAction::REMOVE: return "REMOVE"; break;
		default: return "UNKNOWN";
		}
	}
}