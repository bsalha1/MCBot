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
		mcbot::Vector<double> location;
		float yaw;
		float pitch;

	public:
		Entity(EntityType entity_type, int id);
		Entity(int id);
		Entity();


		mcbot::Vector<double>& get_location();
		void update_location(mcbot::Vector<double> location);
		void update_location(mcbot::Vector<int> location);
		void update_motion(mcbot::Vector<double> motion);
		void update_rotation(float yaw, float pitch);
		void update_yaw(float yaw);

		void die();

		void set_id(int id);

		int get_id();
		EntityType get_entity_type();
		bool is_dead();
	};
}

