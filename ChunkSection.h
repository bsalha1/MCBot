#pragma once

#include <iostream>
#include <array>

#include "Buffer.h"
#include "Vector.h"

namespace McBot
{
	class ChunkSection
	{
	private:
		std::array<uint16_t, 4096> block_ids{ 0 };
		std::array<uint8_t, 2048> emitted_light{ 0 };
		std::array<uint8_t, 2048> sky_light{ 0 };
		std::array<uint8_t, 256> biome_index{ 0 };

	public:
		int GetBlockID(int x, int y, int z);
		int GetBlockID(Vector<double> location);

		void SetBlockID(int x, int y, int z, int block_id);
		void SetBlockIDs(Buffer<uint16_t> block_ids);
		void SetEmittedLight(Buffer<uint8_t> emitted_light);
		void SetSkyLight(Buffer<uint8_t> sky_light);
		void SetBiomeIndex(Buffer<uint8_t> biome_index);
	};
}

