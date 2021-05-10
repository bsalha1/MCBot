#pragma once

#include <iostream>
#include <array>

#include "Buffer.h"
#include "Vector.h"

namespace mcbot
{
	class ChunkSection
	{
	private:
		std::array<uint16_t, 4096> block_ids{ 0 };
		std::array<uint8_t, 2048> emitted_light{ 0 };
		std::array<uint8_t, 2048> sky_light{ 0 };
		std::array<uint8_t, 256> biome_index{ 0 };

	public:
		int get_block_id(int x, int y, int z);
		int get_block_id(mcbot::Vector<double> location);
		void set_block_id(int x, int y, int z, int block_id);
		void set_block_ids(mcbot::Buffer<uint16_t> block_ids);
		void set_emitted_light(mcbot::Buffer<uint8_t> emitted_light);
		void set_sky_light(mcbot::Buffer<uint8_t> sky_light);
		void set_biome_index(mcbot::Buffer<uint8_t> biome_index);
	};
}

