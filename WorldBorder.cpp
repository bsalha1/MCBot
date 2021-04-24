#include "WorldBorder.h"

mcbot::WorldBorder::WorldBorder()
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

void mcbot::WorldBorder::set_radius(double radius)
{
	this->radius = radius;
}

void mcbot::WorldBorder::set_old_radius(double old_radius)
{
	this->old_radius = old_radius;
}

void mcbot::WorldBorder::set_new_radius(double new_radius)
{
	this->new_radius = new_radius;
}

void mcbot::WorldBorder::set_speed(long speed)
{
	this->speed = speed;
}

void mcbot::WorldBorder::set_x(double x)
{
	this->x = x;
}

void mcbot::WorldBorder::set_z(double z)
{
	this->z = z;
}

void mcbot::WorldBorder::set_portal_teleport_boundary(int portal_teleport_boundary)
{
	this->portal_teleport_boundary = portal_teleport_boundary;
}

void mcbot::WorldBorder::set_warning_time(int warning_time)
{
	this->warning_time = warning_time;
}

void mcbot::WorldBorder::set_warning_blocks(int warning_blocks)
{
	this->warning_blocks = warning_blocks;
}
