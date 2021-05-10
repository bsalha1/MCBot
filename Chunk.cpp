#include "Chunk.h"

mcbot::Chunk::Chunk()
{
    this->x = 0;
    this->z = 0;
    this->primary_bit_mask = 0;
    this->num_chunk_sections = 0;
}

mcbot::Chunk::Chunk(int x, int z, uint16_t primary_bit_mask)
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

int mcbot::Chunk::get_block_id(int x, int y, int z)
{
    ChunkSection section = this->chunk_sections[y >> 4];
    return section.get_block_id(x, y, z);
}

int mcbot::Chunk::get_block_id(mcbot::Vector<int> location)
{
    return this->get_block_id(location.get_x(), location.get_y(), location.get_z());
}

int mcbot::Chunk::get_block_id(mcbot::Vector<double> location)
{
    int x = (int)floor(location.get_x());
    int y = (int)floor(location.get_y());
    int z = (int)floor(location.get_z());

    return this->get_block_id(x, y, z);
}

void mcbot::Chunk::update_block(int x, int y, int z, int block_id)
{
    ChunkSection& section = this->chunk_sections[y >> 4];
    section.set_block_id(x, y, z, block_id);
}

void mcbot::Chunk::update_block(mcbot::Vector<int> location, int block_id)
{
    ChunkSection& section = this->chunk_sections[location.get_y() >> 4];
    section.set_block_id(location.get_x(), location.get_y(), location.get_z(), block_id);
}

void mcbot::Chunk::add_section(int i, ChunkSection section)
{
	this->chunk_sections[i] = section;
}

void mcbot::Chunk::set_biome_index(std::array<uint8_t, 256> biome_index)
{
    this->biome_index = biome_index;
}

int mcbot::Chunk::get_num_sections()
{
    return this->num_chunk_sections;
}

uint16_t mcbot::Chunk::get_primary_bit_mask()
{
    return this->primary_bit_mask;
}

int mcbot::Chunk::get_x()
{
    return this->x;
}

int mcbot::Chunk::get_z()
{
    return this->z;
}
