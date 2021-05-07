#include "Entity.h"

mcbot::Entity::Entity(EntityType entity_type, int id, mcbot::Vector<int> initial_location)
{
	this->entity_type = entity_type;
	this->id = id;
	this->location = mcbot::Vector<double>();
	this->location.set_x(initial_location.get_x());
	this->location.set_y(initial_location.get_y());
	this->location.set_z(initial_location.get_z());
}

mcbot::Entity::Entity()
{
	this->entity_type = EntityType::UNKNOWN;
	this->id = -1;
}

mcbot::EntityType mcbot::Entity::get_entity_type()
{
	return this->entity_type;
}

mcbot::Vector<double> mcbot::Entity::get_location()
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
