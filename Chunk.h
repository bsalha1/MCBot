#pragma once

#include <list>
#include <array>

#include "ChunkSection.h"
#include "Buffer.h"

namespace mcbot
{
	class Chunk
	{
	private:
		int x;
		int z;
		int num_chunk_sections;
		uint16_t primary_bit_mask;
		std::array<uint8_t, 256> biome_index;
		std::array<ChunkSection, 16> chunk_sections;

	public:
		Chunk();
		Chunk(int x, int z, uint16_t primary_bit_mask);

		int get_block_id(int x, int y, int z);
		int get_block_id(mcbot::Vector<double> location);

		void add_section(int i, ChunkSection section);

		void set_biome_index(std::array<uint8_t, 256> biome_index);

		int get_num_sections();
		uint16_t get_primary_bit_mask();
		int get_x();
		int get_z();
	};
}

