#pragma once

namespace mcbot
{
	enum class VillagerType {
		DESERT = 0,
		JUNGLE = 1,
		PLAINS = 2,
		SAVANNA = 3,
		SNOW = 4,
		SWAMP = 5,
		TAIGA = 6
	};

	enum class VillagerProfession {
		NONE = 0,
		ARMORER = 1,
		BUTCHER = 2,
		CARTOGRAPHER = 3,
		CLERIC = 4,
		FARMER = 5,
		FISHERMAN = 6,
		FLETCHER = 7,
		LEATHER_WORKER = 8,
		LIBRARIAN = 9,
		MASON = 10,
		NITWIT = 11,
		SHEPHERD = 12,
		TOOLSMITH = 13,
		WEAPONSMITH = 14
	};

	class VillagerData
	{
	private:
		mcbot::VillagerType villager_type;
		mcbot::VillagerProfession villager_profession;
		int level;

	public:
		VillagerData(int villager_type, int villager_profession, int level);
	};
}

