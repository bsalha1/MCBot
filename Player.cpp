#include "Player.h"

mcbot::Player::Player(mcbot::UUID uuid, std::string name,
	std::list<mcbot::PlayerProperty> properties, 
	mcbot::Gamemode gamemode, 
	int ping, std::string display_name)
{
	this->uuid = uuid;
	this->name = name;
	this->properties = properties;
	this->gamemode = gamemode;
	this->ping = ping;
	this->display_name = display_name;
}

mcbot::Player::Player(mcbot::UUID uuid)
{
	this->uuid = uuid;
	this->name = "";
	this->properties = std::list<PlayerProperty>();
	this->gamemode = mcbot::Gamemode::SURVIVAL;
	this->ping = -1;
	this->display_name = "";
}

mcbot::Player::Player()
{
	this->uuid = UUID();
	this->name = "";
	this->properties = std::list<PlayerProperty>();
	this->gamemode = mcbot::Gamemode::SURVIVAL;
	this->ping = -1;
	this->display_name = "";
}

void mcbot::Player::update_position(mcbot::Vector<double> position)
{
	this->position = position;
}

void mcbot::Player::update_rotation(float yaw, float pitch)
{
	this->yaw = yaw;
	this->pitch = pitch;
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
	this->display_name = display_name;
}

void mcbot::Player::set_name(std::string name)
{
	this->name = name;
}

void mcbot::Player::set_uuid(mcbot::UUID uuid)
{
	this->uuid = uuid;
}

void mcbot::Player::set_properties(std::list<mcbot::PlayerProperty> properties)
{
	this->properties = properties;
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

mcbot::Vector<double> mcbot::Player::get_position()
{
	return this->position;
}

int mcbot::Player::get_ping()
{
	return this->ping;
}

float mcbot::Player::get_pitch()
{
	return this->pitch;
}

float mcbot::Player::get_yaw()
{
	return this->yaw;
}

bool mcbot::Player::has_display_name()
{
	return this->display_name.compare(this->name) != 0;
}

std::string mcbot::Player::get_display_name()
{
	return this->display_name;
}
