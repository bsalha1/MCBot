#pragma once
namespace mcbot
{
	class WorldBorder
	{
	private:
		double radius;
		double old_radius;
		double new_radius;
		long speed;
		double x;
		double z;
		int portal_teleport_boundary;
		int warning_time;
		int warning_blocks;
	public:
		WorldBorder();

		void set_radius(double radius);

		void set_old_radius(double old_radius);

		void set_new_radius(double new_radius);

		void set_speed(long speed);

		void set_x(double x);

		void set_z(double y);

		void set_portal_teleport_boundary(int portal_teleport_boundary);

		void set_warning_time(int warning_time);

		void set_warning_blocks(int warning_blocks);
	};
}

