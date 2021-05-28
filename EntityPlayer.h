#pragma once

#include <iostream>
#include <list>
#include <array>

#include "EntityLiving.h"
#include "Enums.h"
#include "PlayerProperty.h"
#include "UUID.h"
#include "Slot.h"
#include "Vector.h"

namespace mcbot
{
	class EntityPlayer : public EntityLiving 
	{
	private:
		UUID uuid;
		std::string name;
		std::list<PlayerProperty> properties;
		Gamemode gamemode;
		std::array<Slot, 45> inventory;
		int ping; // in milliseconds
		std::string display_name;

	public:
		EntityPlayer(int entity_id, UUID uuid, std::string name,
			std::list<PlayerProperty> properties,
			Gamemode gamemode,
			int ping, std::string display_name);

		EntityPlayer(int entity_id, UUID uuid);

		EntityPlayer();

		void SetGamemode(Gamemode gamemode);
		void SetPing(int ping);
		void SetDisplayName(std::string display_name);
		void SetName(std::string name);
		void SetUUID(UUID uuid);
		void SetProperties(std::list<PlayerProperty> properties);
		void SetInventory(std::array<Slot, 45> inventory);

		std::array<Slot, 45> GetInventory();
		UUID GetUUID();
		std::string GetName();
		std::list<PlayerProperty> GetProperties();
		Gamemode GetGamemode();
		Vector<double> GetLocation();
		int GetPing();
		float GetPitch();
		float GetYaw();
		bool HasDisplayName();
		std::string GetDisplayName();

		friend bool operator==(const EntityPlayer& lhs, const EntityPlayer& rhs);
		friend bool operator!=(const EntityPlayer& lhs, const EntityPlayer& rhs);
	};
}

