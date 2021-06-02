#pragma once

#include "Enums.h"
#include "Vector.h"

namespace mcbot
{
	class Entity
	{
	private:
		EntityType entity_type;
		int id;
		bool dead;

	protected:
		Vector<double> location;
		float yaw;
		float pitch;

	public:
		Entity(EntityType entity_type, int id);
		Entity(int id);
		Entity();


		Vector<double>& GetLocation();
		void UpdateLocation(Vector<double> location);
		void UpdateLocation(Vector<int> location);
		void UpdateMotion(Vector<double> motion);
		void UpdateRotation(float yaw, float pitch);
		void UpdateYaw(float yaw);
		void UpdatePitch(float pitch);

		void Die();

		void SetID(int id);

		int GetID();
		EntityType GetEntityType();
		bool IsDead();
	};
}

