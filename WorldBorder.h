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

		void SetRadius(double radius);

		void SetOldRadius(double old_radius);

		void SetNewRadius(double new_radius);

		void SetSpeed(long speed);

		void SetX(double x);

		void SetZ(double y);

		void SetPortalTeleportBoundary(int portal_teleport_boundary);

		void SetWarningTime(int warning_time);

		void SetWarningBlocks(int warning_blocks);
	};
}

