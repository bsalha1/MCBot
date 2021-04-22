#include <iostream>

#include "Enums.h"

std::ostream& operator<<(std::ostream& os, mcbot::State c)
{
	switch (c)
	{
	case mcbot::State::HANDSHAKE: os << "HANDSHAKE"; break;
	case mcbot::State::STATUS: os << "STATUS"; break;
	case mcbot::State::LOGIN: os << "LOGIN"; break;
	case mcbot::State::PLAY: os << "PLAY"; break;
	default: os.setstate(std::ios_base::failbit);
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, mcbot::Gamemode c)
{
	switch (c)
	{
	case mcbot::Gamemode::SURVIVAL: os << "SURVIVAL"; break;
	case mcbot::Gamemode::CREATIVE: os << "CREATIVE"; break;
	case mcbot::Gamemode::ADVENTURE: os << "ADVENTURE"; break;
	case mcbot::Gamemode::SPECTATOR: os << "SPECTATOR"; break;
	default: os.setstate(std::ios_base::failbit);
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, mcbot::Dimension c)
{
	switch (c)
	{
	case mcbot::Dimension::NETHER: os << "NETHER"; break;
	case mcbot::Dimension::OVERWORLD: os << "OVERWORLD"; break;
	case mcbot::Dimension::END: os << "END"; break;
	default: os.setstate(std::ios_base::failbit);
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, mcbot::Difficulty c)
{
	switch (c)
	{
	case mcbot::Difficulty::PEACEFUL: os << "PEACEFUL"; break;
	case mcbot::Difficulty::EASY: os << "EASY"; break;
	case mcbot::Difficulty::NORMAL: os << "NORMAL"; break;
	case mcbot::Difficulty::HARD: os << "HARD"; break;
	default: os.setstate(std::ios_base::failbit);
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, mcbot::FriendlyFire c)
{
	switch (c)
	{
	case mcbot::FriendlyFire::OFF: os << "OFF"; break;
	case mcbot::FriendlyFire::ON: os << "ON"; break;
	case mcbot::FriendlyFire::SEEING_INVISIBLE: os << "SEEING_INVISIBLE"; break;
	default: os.setstate(std::ios_base::failbit);
	}
	return os;
}
