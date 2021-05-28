#include "EntityPlayer.h"

namespace mcbot
{

	EntityPlayer::EntityPlayer(int entity_id, UUID uuid, std::string name,
		std::list<PlayerProperty> properties,
		Gamemode gamemode,
		int ping, std::string display_name) : EntityLiving(EntityType::PLAYER, entity_id)
	{
		this->uuid = uuid;
		this->name = name;
		this->properties = properties;
		this->gamemode = gamemode;
		this->ping = ping;
		this->display_name = display_name;
	}

	EntityPlayer::EntityPlayer(int entity_id, UUID uuid) : EntityLiving(EntityType::PLAYER, entity_id)
	{
		this->uuid = uuid;
		this->name = "";
		this->properties = std::list<PlayerProperty>();
		this->gamemode = Gamemode::SURVIVAL;
		this->ping = -1;
		this->display_name = "";
	}

	EntityPlayer::EntityPlayer() : EntityLiving(EntityType::PLAYER, -1)
	{
		this->uuid = UUID();
		this->name = "";
		this->properties = std::list<PlayerProperty>();
		this->gamemode = Gamemode::SURVIVAL;
		this->ping = -1;
		this->display_name = "";
	}

	void EntityPlayer::SetGamemode(Gamemode gamemode)
	{
		this->gamemode = gamemode;
	}

	void EntityPlayer::SetPing(int ping)
	{
		this->ping = ping;
	}

	void EntityPlayer::SetDisplayName(std::string display_name)
	{
		this->display_name = display_name;
	}

	void EntityPlayer::SetName(std::string name)
	{
		this->name = name;
	}

	void EntityPlayer::SetUUID(UUID uuid)
	{
		this->uuid = uuid;
	}

	void EntityPlayer::SetProperties(std::list<PlayerProperty> properties)
	{
		this->properties = properties;
	}

	void EntityPlayer::SetInventory(std::array<Slot, 45> inventory)
	{
		this->inventory = inventory;
	}

	std::array<Slot, 45> EntityPlayer::GetInventory()
	{
		return this->inventory;
	}

	UUID EntityPlayer::GetUUID()
	{
		return this->uuid;
	}

	std::string EntityPlayer::GetName()
	{
		return this->name;
	}

	std::list<PlayerProperty> EntityPlayer::GetProperties()
	{
		return this->properties;
	}

	Gamemode EntityPlayer::GetGamemode()
	{
		return this->gamemode;
	}

	Vector<double> EntityPlayer::GetLocation()
	{
		return this->location;
	}

	int EntityPlayer::GetPing()
	{
		return this->ping;
	}

	float EntityPlayer::GetPitch()
	{
		return this->pitch;
	}

	float EntityPlayer::GetYaw()
	{
		return this->yaw;
	}

	bool EntityPlayer::HasDisplayName()
	{
		return this->display_name.compare(this->name) != 0;
	}

	std::string EntityPlayer::GetDisplayName()
	{
		return this->display_name;
	}

	bool operator==(const EntityPlayer& lhs, const EntityPlayer& rhs)
	{
		return lhs.uuid == rhs.uuid;
	}

	bool operator!=(const EntityPlayer& lhs, const EntityPlayer& rhs)
	{
		return !(lhs == rhs);
	}
}
