#pragma once

#include <list>
#include <array>

#include "ChunkSection.h"
#include "Buffer.h"

namespace McBot
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

		int GetBlockID(int x, int y, int z);
		int GetBlockID(Vector<int> location);
		int GetBlockID(Vector<double> location);

		void UpdateBlock(int x, int y, int z, int block_id);
		void UpdateBlock(Vector<int> location, int block_id);

		void AddSection(int i, ChunkSection section);

		std::list<Vector<int>> GetBlockCoordinates(int block_id);

		void SetBiomeIndex(std::array<uint8_t, 256> biome_index);

		// Local Variable Access //
		int GetNumSections();
		uint16_t GetPrimaryBitMask();
		int GetX();
		int GetZ();
	};
}

