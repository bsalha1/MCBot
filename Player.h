#pragma once

#include <iostream>
#include <list>

#include "PlayerProperty.h"
#include "UUID.h"
#include "Enums.h"

namespace mcbot
{
	class Player
	{
	private:
		mcbot::UUID uuid;
		std::string name;
		std::list<mcbot::PlayerProperty> properties;
		mcbot::Gamemode gamemode;
		int ping; // in milliseconds
		bool has_a_display_name;
		std::string display_name;

	public:
		Player(mcbot::UUID uuid, std::string name,
			std::list<mcbot::PlayerProperty> properties,
			mcbot::Gamemode gamemode,
			int ping, bool has_a_display_name, std::string display_name);

		Player(mcbot::UUID uuid);

		Player();

		void set_gamemode(mcbot::Gamemode gamemode);

		void set_ping(int ping);

		void set_display_name(std::string display_name);

		mcbot::UUID get_uuid();

		std::string get_name();

		std::list<mcbot::PlayerProperty> get_properties();

		mcbot::Gamemode get_gamemode();

		int get_ping();

		bool has_display_name();

		std::string get_display_name();
	};
}

