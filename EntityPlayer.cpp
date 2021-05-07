#include "EntityPlayer.h"

mcbot::EntityPlayer::EntityPlayer(int entity_id, mcbot::UUID uuid, std::string name,
	std::list<mcbot::PlayerProperty> properties, 
	mcbot::Gamemode gamemode, 
	int ping, std::string display_name) : EntityLiving(EntityType::PLAYER, entity_id, mcbot::Vector<int>())
{
	this->uuid = uuid;
	this->name = name;
	this->properties = properties;
	this->gamemode = gamemode;
	this->ping = ping;
	this->display_name = display_name;
	this->yaw = 0;
	this->pitch = 0;
}

mcbot::EntityPlayer::EntityPlayer(int entity_id, UUID uuid) : EntityLiving(EntityType::PLAYER, entity_id, mcbot::Vector<int>())
{
	this->uuid = uuid;
	this->name = "";
	this->properties = std::list<PlayerProperty>();
	this->gamemode = mcbot::Gamemode::SURVIVAL;
	this->ping = -1;
	this->display_name = "";
	this->yaw = 0;
	this->pitch = 0;
}

mcbot::EntityPlayer::EntityPlayer() : EntityLiving(EntityType::PLAYER, -1, mcbot::Vector<int>())
{
	this->uuid = UUID();
	this->name = "";
	this->properties = std::list<PlayerProperty>();
	this->gamemode = mcbot::Gamemode::SURVIVAL;
	this->ping = -1;
	this->display_name = "";
	this->yaw = 0;
	this->pitch = 0;
}

void mcbot::EntityPlayer::update_location(mcbot::Vector<double> location)
{
	this->location = location;
}

void mcbot::EntityPlayer::update_location(mcbot::Vector<int> location)
{
	this->location.set_x(location.get_x());
	this->location.set_y(location.get_y());
	this->location.set_z(location.get_z());
}

void mcbot::EntityPlayer::update_rotation(float yaw, float pitch)
{
	this->yaw = yaw;
	this->pitch = pitch;
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
