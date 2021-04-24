#include "Player.h"

mcbot::Player::Player(mcbot::UUID uuid, std::string name,
	std::list<mcbot::PlayerProperty> properties, 
	mcbot::Gamemode gamemode, 
	int ping, bool has_a_display_name, std::string display_name)
{
	this->uuid = uuid;
	this->name = name;
	this->properties = properties;
	this->gamemode = gamemode;
	this->ping = ping;
	this->has_a_display_name = has_a_display_name;

	if (has_a_display_name)
	{
		this->display_name = display_name;
	}
	else
	{
		this->display_name = "";
	}
}

mcbot::Player::Player(mcbot::UUID uuid)
{
	this->uuid = uuid;
	this->name = "";
	this->properties = std::list<PlayerProperty>();
	this->gamemode = mcbot::Gamemode::SURVIVAL;
	this->ping = -1;
	this->has_a_display_name = false;
	this->display_name = "";
}

mcbot::Player::Player()
{
	this->uuid = UUID();
	this->name = "";
	this->properties = std::list<PlayerProperty>();
	this->gamemode = mcbot::Gamemode::SURVIVAL;
	this->ping = -1;
	this->has_a_display_name = false;
	this->display_name = "";
}

void mcbot::Player::set_gamemode(mcbot::Gamemode gamemode)
{
	this->gamemode = gamemode;
}

void mcbot::Player::set_ping(int ping)
{
	this->ping = ping;
}

void mcbot::Player::set_display_name(std::string display_name)
{
	this->has_a_display_name = true;
	this->display_name = display_name;
}

mcbot::UUID mcbot::Player::get_uuid()
{
	return this->uuid;
}

std::string mcbot::Player::get_name()
{
	return this->name;
}

std::list<mcbot::PlayerProperty> mcbot::Player::get_properties()
{
	return this->properties;
}

mcbot::Gamemode mcbot::Player::get_gamemode()
{
	return this->gamemode;
}

int mcbot::Player::get_ping()
{
	return this->ping;
}

bool mcbot::Player::has_display_name()
{
	return this->has_a_display_name;
}

std::string mcbot::Player::get_display_name()
{
	return this->display_name;
}
