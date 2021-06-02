#include "Entity.h"

namespace mcbot
{

	Entity::Entity(EntityType entity_type, int id)
	{
		this->entity_type = entity_type;
		this->id = id;
		this->dead = false;
		this->yaw = 0;
		this->pitch = 0;
	}

	Entity::Entity(int id)
	{
		this->entity_type = EntityType::UNKNOWN;
		this->id = id;
		this->dead = false;
		this->yaw = 0;
		this->pitch = 0;
	}

	Entity::Entity()
	{
		this->entity_type = EntityType::UNKNOWN;
		this->id = -1;
		this->dead = false;
		this->yaw = 0;
		this->pitch = 0;
	}

	void Entity::UpdateLocation(Vector<double> location)
	{
		this->location = location;
	}

	void Entity::UpdateLocation(Vector<int> location)
	{
		this->location.SetX(location.GetX());
		this->location.SetY(location.GetY());
		this->location.SetZ(location.GetZ());
	}

	void Entity::UpdateMotion(Vector<double> motion)
	{
		this->location = this->location + motion;
	}

	void Entity::UpdateRotation(float yaw, float pitch)
	{
		this->yaw = yaw;
		this->pitch = pitch;
	}

	void Entity::UpdateYaw(float yaw)
	{
		this->yaw = yaw;
	}

	void Entity::UpdatePitch(float pitch)
	{
		this->pitch = pitch;
	}

	void Entity::Die()
	{
		this->dead = true;
	}

	Vector<double>& Entity::GetLocation()
	{
		return this->location;
	}

	void Entity::SetID(int id)
	{
		this->id = id;
	}

	int Entity::GetID()
	{
		return id;
	}

	EntityType Entity::GetEntityType()
	{
		return this->entity_type;
	}

	bool Entity::IsDead()
	{
		return this->dead;
	}
}

