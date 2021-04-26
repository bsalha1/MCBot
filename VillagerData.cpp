#include "VillagerData.h"


mcbot::VillagerData::VillagerData(int villager_type, int villager_profession, int level)
{
	this->villager_type = (mcbot::VillagerType) villager_type;
	this->villager_profession = (mcbot::VillagerProfession) villager_profession;
	this->level = level;
}
