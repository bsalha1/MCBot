#include "ChunkSection.h"

namespace mcbot
{

	int ChunkSection::GetBlockID(int x, int y, int z)
	{
		x &= 0xF;
		y &= 0xF;
		z &= 0xF;

		return this->block_ids[y << 8 | z << 4 | x] >> 4;
	}

	int ChunkSection::GetBlockID(Vector<double> location)
	{
		int x = (int)floor(location.GetX());
		int y = (int)floor(location.GetY());
		int z = (int)floor(location.GetZ());

		return this->GetBlockID(x, y, z);
	}

	void ChunkSection::SetBlockID(int x, int y, int z, int block_id)
	{
		x &= 0xF;
		y &= 0xF;
		z &= 0xF;

		this->block_ids[y << 8 | z << 4 | x] = block_id << 4;
	}

	void ChunkSection::SetBlockIDs(Buffer<uint16_t> block_ids)
	{
		this->block_ids = block_ids.to_array<4096>();
	}

	void ChunkSection::SetEmittedLight(Buffer<uint8_t> emitted_light)
	{
		this->emitted_light = emitted_light.to_array<2048>();
	}

	void ChunkSection::SetSkyLight(Buffer<uint8_t> sky_light)
	{
		this->sky_light = sky_light.to_array<2048>();
	}

	void ChunkSection::SetBiomeIndex(Buffer<uint8_t> biome_index)
	{
		this->biome_index = biome_index.to_array<256>();
	}
}
