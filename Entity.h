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

	protected:
		mcbot::Vector<double> location;

	public:
		Entity(EntityType entity_type, int id, mcbot::Vector<int> initial_location);

		Entity();

		EntityType get_entity_type();

		mcbot::Vector<double> get_location();

		void set_id(int id);

		int get_id();
	};
}

