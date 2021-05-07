#include "ChunkSection.h"

int mcbot::ChunkSection::get_block_id(int x, int y, int z)
{
	x &= 0xF;
	y &= 0xF;
	z &= 0xF;

	return this->block_ids[y << 8 | z << 4 | x] >> 4;
}

int mcbot::ChunkSection::get_block_id(mcbot::Vector<double> location)
{
	int x = (int)floor(location.get_x());
	int y = (int)floor(location.get_y());
	int z = (int)floor(location.get_z()); 
	
	return this->get_block_id(x, y, z);
}

void mcbot::ChunkSection::set_block_ids(mcbot::Buffer<uint16_t> block_ids)
{
	this->block_ids = block_ids.to_array<4096>();
}

void mcbot::ChunkSection::set_emitted_light(mcbot::Buffer<uint8_t> emitted_light)
{
	this->emitted_light = emitted_light.to_array<2048>();
}

void mcbot::ChunkSection::set_sky_light(mcbot::Buffer<uint8_t> sky_light)
{
	this->sky_light = sky_light.to_array<2048>();
}

void mcbot::ChunkSection::set_biome_index(mcbot::Buffer<uint8_t> biome_index)
{
	this->biome_index = biome_index.to_array<256>();
}
