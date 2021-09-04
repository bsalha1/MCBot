#pragma once

#include "Entity.h"

namespace McBot
{
	class EntityLiving : public Entity
	{
	public:
		EntityLiving(EntityType entity_type, int id);
	};
}

