#include "VillagerData.h"

namespace McBot
{

	VillagerData::VillagerData(int villager_type, int villager_profession, int level)
	{
		this->villager_type = (VillagerType) villager_type;
		this->villager_profession = (VillagerProfession) villager_profession;
		this->level = level;
	}
}
