#include "Entity.h"

mcbot::Entity::Entity(EntityType entity_type, int id)
{
	this->entity_type = entity_type;
	this->id = id;
	this->dead = false;
}

mcbot::Entity::Entity(int id)
{
	this->entity_type = EntityType::UNKNOWN;
	this->id = id;
	this->dead = false;
}

mcbot::Entity::Entity()
{
	this->entity_type = EntityType::UNKNOWN;
	this->id = -1;
	this->dead = false;
}

void mcbot::Entity::update_location(mcbot::Vector<double> location)
{
	this->location = location;
}

void mcbot::Entity::update_location(mcbot::Vector<int> location)
{
	this->location.set_x(location.get_x());
	this->location.set_y(location.get_y());
	this->location.set_z(location.get_z());
}

void mcbot::Entity::update_motion(mcbot::Vector<double> motion)
{
	this->location = this->location + motion;
}

void mcbot::Entity::update_rotation(float yaw, float pitch)
{
	this->yaw = yaw;
	this->pitch = pitch;
}

void mcbot::Entity::update_yaw(float yaw)
{
	this->yaw = yaw;
}

void mcbot::Entity::die()
{
	this->dead = true;
}

mcbot::Vector<double>& mcbot::Entity::get_location()
{
	return this->location;
}

void mcbot::Entity::set_id(int id)
{
	this->id = id;
}

int mcbot::Entity::get_id()
{
	return id;
}

mcbot::EntityType mcbot::Entity::get_entity_type()
{
	return this->entity_type;
}

bool mcbot::Entity::is_dead()
{
	return this->dead;
}

