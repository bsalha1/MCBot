#include "WorldBorder.h"

namespace McBot
{

	WorldBorder::WorldBorder()
	{
		this->radius = 0;
		this->old_radius = 0;
		this->new_radius = 0;
		this->speed = 0;
		this->x = 0;
		this->z = 0;
		this->portal_teleport_boundary = 0;
		this->warning_time = 0;
		this->warning_blocks = 0;
	}

	void WorldBorder::SetRadius(double radius)
	{
		this->radius = radius;
	}

	void WorldBorder::SetOldRadius(double old_radius)
	{
		this->old_radius = old_radius;
	}

	void WorldBorder::SetNewRadius(double new_radius)
	{
		this->new_radius = new_radius;
	}

	void WorldBorder::SetSpeed(long speed)
	{
		this->speed = speed;
	}

	void WorldBorder::SetX(double x)
	{
		this->x = x;
	}

	void WorldBorder::SetZ(double z)
	{
		this->z = z;
	}

	void WorldBorder::SetPortalTeleportBoundary(int portal_teleport_boundary)
	{
		this->portal_teleport_boundary = portal_teleport_boundary;
	}

	void WorldBorder::SetWarningTime(int warning_time)
	{
		this->warning_time = warning_time;
	}

	void WorldBorder::SetWarningBlocks(int warning_blocks)
	{
		this->warning_blocks = warning_blocks;
	}
}
