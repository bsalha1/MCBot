#pragma once

#include "Entity.h"

namespace mcbot
{
	class EntityLiving : public Entity
	{
	public:
		EntityLiving(EntityType entity_type, int id);
	};
}

