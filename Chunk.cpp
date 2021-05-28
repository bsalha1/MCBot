#include "Chunk.h"

namespace mcbot
{
    Chunk::Chunk()
    {
        this->x = 0;
        this->z = 0;
        this->primary_bit_mask = 0;
        this->num_chunk_sections = 0;
    }

    Chunk::Chunk(int x, int z, uint16_t primary_bit_mask)
    {
        this->x = x;
        this->z = z;
        this->primary_bit_mask = primary_bit_mask;

        int i = primary_bit_mask;

        i = i - ((i >> 1) & 0x55555555);
        i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
        i = (i + (i >> 4)) & 0x0f0f0f0f;
        i = i + (i >> 8);
        i = i + (i >> 16);

        this->num_chunk_sections = i & 0x3f;
    }

    int Chunk::GetBlockID(int x, int y, int z)
    {
        ChunkSection section = this->chunk_sections[y >> 4];
        return section.GetBlockID(x, y, z);
    }

    int Chunk::GetBlockID(Vector<int> location)
    {
        return this->GetBlockID(location.GetX(), location.GetY(), location.GetZ());
    }

    int Chunk::GetBlockID(Vector<double> location)
    {
        int x = (int)floor(location.GetX());
        int y = (int)floor(location.GetY());
        int z = (int)floor(location.GetZ());

        return this->GetBlockID(x, y, z);
    }

    void Chunk::UpdateBlock(int x, int y, int z, int block_id)
    {
        ChunkSection& section = this->chunk_sections[y >> 4];
        section.SetBlockID(x, y, z, block_id);
    }

    void Chunk::UpdateBlock(Vector<int> location, int block_id)
    {
        ChunkSection& section = this->chunk_sections[location.GetY() >> 4];
        section.SetBlockID(location.GetX(), location.GetY(), location.GetZ(), block_id);
    }

    void Chunk::AddSection(int i, ChunkSection section)
    {
        this->chunk_sections[i] = section;
    }

    std::list<Vector<int>> Chunk::GetBlockCoordinates(int block_id)
    {
        std::list<Vector<int>> coordinates;

        int x = this->x << 4;
        int y = 0;
        int z = this->z << 4;

        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 256; j++)
            {
                for (int k = 0; k < 16; k++)
                {
                    int id = this->GetBlockID(x + i, y + j, z + k);
                    if (id == block_id)
                    {
                        coordinates.push_back(Vector<int>(x + i, y + j, z + k));
                    }
                }
            }
        }

        return coordinates;
    }

    void Chunk::SetBiomeIndex(std::array<uint8_t, 256> biome_index)
    {
        this->biome_index = biome_index;
    }

    int Chunk::GetNumSections()
    {
        return this->num_chunk_sections;
    }

    uint16_t Chunk::GetPrimaryBitMask()
    {
        return this->primary_bit_mask;
    }

    int Chunk::GetX()
    {
        return this->x;
    }

    int Chunk::GetZ()
    {
        return this->z;
    }
}