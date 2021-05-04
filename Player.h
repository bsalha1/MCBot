#pragma once

#include <iostream>
#include <list>

#include "PlayerProperty.h"
#include "UUID.h"
#include "Enums.h"
#include "Vector.h"

namespace mcbot
{
	class Player
	{
	private:
		mcbot::UUID uuid;
		std::string name;
		std::list<PlayerProperty> properties;
		mcbot::Gamemode gamemode;
		int ping; // in milliseconds
		std::string display_name;

		Vector<double> position;
		float yaw;
		float pitch;

	public:
		Player(UUID uuid, std::string name,
			std::list<PlayerProperty> properties,
			Gamemode gamemode,
			int ping, std::string display_name);

		Player(mcbot::UUID uuid);

		Player();

		void update_position(mcbot::Vector<double> position);

		void update_rotation(float yaw, float pitch);

		void set_gamemode(Gamemode gamemode);

		void set_ping(int ping);

		void set_display_name(std::string display_name);

		void set_name(std::string name);

		void set_uuid(UUID uuid);

		void set_properties(std::list<PlayerProperty> properties);

		UUID get_uuid();

		std::string get_name();

		std::list<mcbot::PlayerProperty> get_properties();

		mcbot::Gamemode get_gamemode();

		mcbot::Vector<double> get_position();

		int get_ping();

		float get_pitch();

		float get_yaw();

		bool has_display_name();

		std::string get_display_name();
	};
}

