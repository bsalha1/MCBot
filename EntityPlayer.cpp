#include "EntityPlayer.h"

mcbot::EntityPlayer::EntityPlayer(int entity_id, mcbot::UUID uuid, std::string name,
	std::list<mcbot::PlayerProperty> properties, 
	mcbot::Gamemode gamemode, 
	int ping, std::string display_name) : EntityLiving(EntityType::PLAYER, entity_id)
{
	this->uuid = uuid;
	this->name = name;
	this->properties = properties;
	this->gamemode = gamemode;
	this->ping = ping;
	this->display_name = display_name;
}

mcbot::EntityPlayer::EntityPlayer(int entity_id, UUID uuid) : EntityLiving(EntityType::PLAYER, entity_id)
{
	this->uuid = uuid;
	this->name = "";
	this->properties = std::list<PlayerProperty>();
	this->gamemode = mcbot::Gamemode::SURVIVAL;
	this->ping = -1;
	this->display_name = "";
}

mcbot::EntityPlayer::EntityPlayer() : EntityLiving(EntityType::PLAYER, -1)
{
	this->uuid = UUID();
	this->name = "";
	this->properties = std::list<PlayerProperty>();
	this->gamemode = mcbot::Gamemode::SURVIVAL;
	this->ping = -1;
	this->display_name = "";
}

void mcbot::EntityPlayer::set_gamemode(mcbot::Gamemode gamemode)
{
	this->gamemode = gamemode;
}

void mcbot::EntityPlayer::set_ping(int ping)
{
	this->ping = ping;
}

void mcbot::EntityPlayer::set_display_name(std::string display_name)
{
	this->display_name = display_name;
}

void mcbot::EntityPlayer::set_name(std::string name)
{
	this->name = name;
}

void mcbot::EntityPlayer::set_uuid(mcbot::UUID uuid)
{
	this->uuid = uuid;
}

void mcbot::EntityPlayer::set_properties(std::list<mcbot::PlayerProperty> properties)
{
	this->properties = properties;
}

mcbot::UUID mcbot::EntityPlayer::get_uuid()
{
	return this->uuid;
}

std::string mcbot::EntityPlayer::get_name()
{
	return this->name;
}

std::list<mcbot::PlayerProperty> mcbot::EntityPlayer::get_properties()
{
	return this->properties;
}

mcbot::Gamemode mcbot::EntityPlayer::get_gamemode()
{
	return this->gamemode;
}

mcbot::Vector<double> mcbot::EntityPlayer::get_location()
{
	return this->location;
}

int mcbot::EntityPlayer::get_ping()
{
	return this->ping;
}

float mcbot::EntityPlayer::get_pitch()
{
	return this->pitch;
}

float mcbot::EntityPlayer::get_yaw()
{
	return this->yaw;
}

bool mcbot::EntityPlayer::has_display_name()
{
	return this->display_name.compare(this->name) != 0;
}

std::string mcbot::EntityPlayer::get_display_name()
{
	return this->display_name;
}

bool mcbot::operator==(const EntityPlayer& lhs, const EntityPlayer& rhs)
{
	return lhs.uuid == rhs.uuid;
}

bool mcbot::operator!=(const EntityPlayer& lhs, const EntityPlayer& rhs)
{
	return !(lhs == rhs);
}
