#include "PacketDecoder.h"
#include "StringUtils.h"

namespace mcbot
{
    //---- Basic Types ----//
    int32_t PacketDecoder::ReadVarInt(uint8_t* packet, size_t& offset)
    {
        int num_read = 0;
        int result = 0;
        uint8_t read;
        do {
            read = packet[offset + num_read];
            int value = (read & 0b01111111);
            result |= (value << (7 * num_read));

            num_read++;
            if (num_read > 5)
            {
                offset += num_read;
                fprintf(stderr, "VarInt out of bounds");
                return -1;
            }
        } while ((read & 0b10000000) != 0);

        offset += num_read;
        return result;
    }

    int64_t PacketDecoder::ReadVarLong(uint8_t* packet, size_t& offset)
    {
        int num_read = 0;
        long result = 0;
        uint8_t read;
        do {
            read = packet[offset + num_read];
            long value = (read & 0b01111111);
            result |= (value << (7 * num_read));

            num_read++;
            if (num_read > 10)
            {
                offset += num_read;
                fprintf(stderr, "VarLong out of bounds");
            }
        } while ((read & 0b10000000) != 0);

        return result;
    }

    uint32_t PacketDecoder::ReadInt(uint8_t* packet, size_t& offset)
    {
        uint8_t byte4 = packet[offset++];
        uint8_t byte3 = packet[offset++];
        uint8_t byte2 = packet[offset++];
        uint8_t byte1 = packet[offset++];

        uint32_t result =
            byte4 << 24 |
            byte3 << 16 |
            byte2 << 8 |
            byte1 << 0;
        return result;
    }

    uint16_t PacketDecoder::ReadShort(uint8_t* packet, size_t& offset)
    {
        uint8_t byte2 = packet[offset++];
        uint8_t byte1 = packet[offset++];

        uint16_t result =
            byte2 << 8 |
            byte1 << 0;
        return result;
    }

    uint16_t PacketDecoder::ReadShortLittleEndian(uint8_t* packet, size_t& offset)
    {
        uint8_t byte2 = packet[offset++];
        uint8_t byte1 = packet[offset++];

        uint16_t result =
            byte2 << 0 |
            byte1 << 8;
        return result;
    }

    uint64_t PacketDecoder::ReadLong(uint8_t* packet, size_t& offset)
    {
        uint8_t byte8 = packet[offset++];
        uint8_t byte7 = packet[offset++];
        uint8_t byte6 = packet[offset++];
        uint8_t byte5 = packet[offset++];
        uint8_t byte4 = packet[offset++];
        uint8_t byte3 = packet[offset++];
        uint8_t byte2 = packet[offset++];
        uint8_t byte1 = packet[offset++];

        uint64_t result =
            ((uint64_t)byte8) << 56 |
            ((uint64_t)byte7) << 48 |
            ((uint64_t)byte6) << 40 |
            ((uint64_t)byte5) << 32 |
            ((uint64_t)byte4) << 24 |
            ((uint64_t)byte3) << 16 |
            ((uint64_t)byte2) << 8 |
            ((uint64_t)byte1) << 0;
        return result;
    }

    uint8_t PacketDecoder::ReadByte(uint8_t* packet, size_t& offset)
    {
        return packet[offset++];
    }

    uint8_t PacketDecoder::PeekByte(uint8_t* packet, size_t offset)
    {
        return packet[offset++];
    }

    float PacketDecoder::ReadFloat(uint8_t* packet, size_t& offset)
    {
        uint32_t bytes = ReadInt(packet, offset);
        return *((float*)&bytes);
    }

    double PacketDecoder::ReadDouble(uint8_t* packet, size_t& offset)
    {
        uint64_t bytes = ReadLong(packet, offset);
        return *((double*)&bytes);
    }

    bool PacketDecoder::ReadBoolean(uint8_t* packet, size_t& offset)
    {
        uint8_t value = packet[offset++];
        bool result = false;
        if (value == 1)
        {
            result = true;
        }
        else if (value == 0)
        {
            result = false;
        }
        else
        {
            std::cerr << "Invalid boolean value read: " << value << std::endl;
        }

        return result;
    }

    std::string PacketDecoder::ReadString(uint8_t* packet, size_t& offset)
    {
        int length = ReadVarInt(packet, offset);
        std::string string = "";
        for (size_t i = 0; i < length; i++)
        {
            string += packet[offset++];
        }
        return string;
    }

    std::string PacketDecoder::ReadString(int length, uint8_t* packet, size_t& offset)
    {
        std::string string = "";
        for (size_t i = 0; i < length; i++)
        {
            string += packet[offset++];
        }
        return string;
    }



    //---- Arrays ----//

    void PacketDecoder::ReadByteArray(uint8_t* bytes, int length, uint8_t* packet, size_t& offset)
    {
        for (int i = 0; i < length; i++)
        {
            bytes[i] = packet[offset++];
        }
    }

    Buffer<uint8_t> PacketDecoder::ReadByteArray(int length, uint8_t* packet, size_t& offset)
    {
        Buffer<uint8_t> buffer = Buffer<uint8_t>(length);
        for (int i = 0; i < length; i++)
        {
            buffer.put(packet[offset++]);
        }
        return buffer;
    }

    Buffer<uint16_t> PacketDecoder::ReadShortArray(int length, uint8_t* packet, size_t& offset)
    {
        Buffer<uint16_t> buffer = Buffer<uint16_t>(length);
        for (int i = 0; i < length; i++)
        {
            buffer.put(ReadShort(packet, offset));
        }
        return buffer;

    }

    Buffer<uint16_t> PacketDecoder::ReadShortLittleEndianArray(int length, uint8_t* packet, size_t& offset)
    {
        Buffer<uint16_t> buffer = Buffer<uint16_t>(length);
        for (int i = 0; i < length; i++)
        {
            buffer.put(ReadShortLittleEndian(packet, offset));
        }
        return buffer;

    }

    Buffer<int> PacketDecoder::ReadIntArray(int length, uint8_t* packet, size_t& offset)
    {
        Buffer<int> buffer = Buffer<int>(length);
        for (int i = 0; i < length; i++)
        {
            buffer.put(ReadInt(packet, offset));
        }
        return buffer;
    }

    Buffer<int> PacketDecoder::ReadVarIntArray(int length, uint8_t* packet, size_t& offset)
    {
        Buffer<int> buffer = Buffer<int>(length);
        for (int i = 0; i < length; i++)
        {
            buffer.put(ReadVarInt(packet, offset));
        }
        return buffer;
    }

    Buffer<long> PacketDecoder::ReadLongArray(int length, uint8_t* packet, size_t& offset)
    {
        Buffer<long> buffer = Buffer<long>(length);
        for (int i = 0; i < length; i++)
        {
            buffer.put(ReadLong(packet, offset));
        }

        return buffer;
    }


    std::list<std::string> PacketDecoder::ReadStringArray(int length, uint8_t* packet, size_t& offset)
    {
        std::list<std::string> strings;

        for (int i = 0; i < length; i++)
        {
            strings.push_back(ReadString(packet, offset));
        }

        return strings;
    }

    std::list<Statistic> PacketDecoder::ReadStatisticArray(int length, uint8_t* packet, size_t& offset)
    {
        std::list<Statistic> statistics;

        for (int i = 0; i < length; i++)
        {
            std::string name = ReadString(packet, offset);
            int value = ReadVarInt(packet, offset);
            statistics.push_back(Statistic(name, value));
        }

        return statistics;
    }

    std::list<PlayerProperty> PacketDecoder::ReadPropertyArray(int length, uint8_t* packet, size_t& offset)
    {
        std::list<PlayerProperty> properties;

        for (int i = 0; i < length; i++)
        {
            std::string name = ReadString(packet, offset);
            std::string value = ReadString(packet, offset);
            bool is_signed = ReadBoolean(packet, offset);
            std::string signature = is_signed ? ReadString(packet, offset) : "";
            properties.push_back(PlayerProperty(name, value, is_signed, signature));
        }

        return properties;
    }

    std::list<Slot> PacketDecoder::ReadSlotArray(int length, uint8_t* packet, size_t& offset)
    {
        std::list<Slot> slots;

        for (int i = 0; i < length; i++)
        {
            slots.push_back(ReadSlot(packet, offset));
        }

        return slots;
    }



    //---- Classes ----//

    UUID PacketDecoder::ReadUUID(uint8_t* packet, size_t& offset)
    {
        char bytes[16] = { 0 };
        for (int i = 0; i < 16; i++)
        {
            bytes[i] = packet[offset++];
        }
        return UUID(bytes);
    }

    Slot PacketDecoder::ReadSlot(uint8_t* packet, size_t& offset)
    {
        short item_id = ReadShort(packet, offset);
        if (item_id >= 0)
        {
            uint8_t item_count = ReadByte(packet, offset);
            short data = ReadShort(packet, offset);
            NBTTagCompound nbt = ReadNBT(packet, offset);
            return Slot(item_id, item_count, data, nbt);
        }
        else
        {
            return Slot();
        }
    }

    Color PacketDecoder::ReadColor(uint8_t* packet, size_t& offset)
    {
        float r = ReadFloat(packet, offset);
        float g = ReadFloat(packet, offset);
        float b = ReadFloat(packet, offset);
        float scale = ReadFloat(packet, offset);

        return Color(r, g, b, scale);
    }

    Particle PacketDecoder::ReadParticle(uint8_t* packet, size_t& offset)
    {
        int id = ReadVarInt(packet, offset);
        switch (id)
        {
        case 3:
        {
            int block_state = ReadVarInt(packet, offset);
            return Particle(id, block_state);
        }

        case 14:
        {
            Color dust_color = ReadColor(packet, offset);
            return Particle(id, dust_color);
        }

        case 23:
        {
            float block_state = ReadVarInt(packet, offset);
            return Particle(id, block_state);
        }

        case 32:
        {
            Slot slot = ReadSlot(packet, offset);
            return Particle(id, slot);
        }
        }

        return Particle(id);
    }

    Position PacketDecoder::ReadPosition(uint8_t* packet, size_t& offset)
    {
        // Parse X Coordinate (26 bit signed integer)
        uint8_t byte4 = packet[offset++];
        uint8_t byte3 = packet[offset++];
        uint8_t byte2 = packet[offset++];
        uint8_t byte1 = packet[offset] & 0xC0; // xx00 0000

        uint32_t x_bits = ((byte4 << 24) | (byte3 << 16) | (byte2 << 8) | (byte1 << 0)) >> 6;
        int32_t x = x_bits & (1 << 25) ? x_bits - (0x3FFFFFF + 1) : x_bits;

        // Parse Y Coordinate (12 bit signed integer)
        byte2 = packet[offset++] & 0x3F;
        byte1 = packet[offset] & 0xFC;

        uint16_t y_bits = ((byte2 << 8) | (byte1 << 0)) >> 2;
        int16_t y = y_bits & (1 << 11) ? y_bits - (0xFFF + 1) : y_bits;

        // Parse Z Coordinate (26 bit signed integer)
        byte4 = packet[offset++] & 0x03;
        byte3 = packet[offset++];
        byte2 = packet[offset++];
        byte1 = packet[offset++];

        uint32_t z_bits = (byte4 << 24) | (byte3 << 16) | (byte2 << 8) | (byte1 << 0);
        int32_t z = z_bits & (1 << 25) ? z_bits - (0x3FFFFFF + 1) : z_bits;

        return Position(x, y, z);
    }

    VillagerData PacketDecoder::ReadVillagerData(uint8_t* packet, size_t& offset)
    {
        int type = ReadVarInt(packet, offset);
        int profession = ReadVarInt(packet, offset);
        int level = ReadVarInt(packet, offset);
        return VillagerData(type, profession, level);
    }

    AttributeModifier PacketDecoder::ReadAttributeModifier(uint8_t* packet, size_t& offset)
    {
        UUID uuid = ReadUUID(packet, offset);
        double amount = ReadDouble(packet, offset);
        uint8_t operation = ReadByte(packet, offset);
        return AttributeModifier(uuid, amount, operation);
    }

    Attribute PacketDecoder::ReadAttribute(uint8_t* packet, size_t& offset)
    {
        std::string key = ReadString(packet, offset);
        double value = ReadDouble(packet, offset);
        int num_modifiers = ReadVarInt(packet, offset);

        std::list<AttributeModifier> modifiers;
        for (int i = 0; i < num_modifiers; i++)
        {
            modifiers.push_back(ReadAttributeModifier(packet, offset));
        }
        return Attribute(key, value, modifiers);
    }

    EntityMetaData PacketDecoder::ReadMetaData(uint8_t* packet, size_t& offset)
    {
        EntityMetaData meta_data;

        uint8_t index = ReadByte(packet, offset);
        while (index != 0x7F)
        {
            int a = index & 0x1F;
            int type = index >> 5;

            switch (type)
            {
            case 0:
            {
                meta_data.AddValue(ReadByte(packet, offset));
                break;
            }

            case 1:
            {
                meta_data.AddValue(ReadShort(packet, offset));
                break;
            }

            case 2:
            {
                meta_data.AddValue(ReadInt(packet, offset));
                break;
            }

            case 3:
            {
                meta_data.AddValue(ReadFloat(packet, offset));
                break;
            }

            case 4:
            {
                meta_data.AddValue(ReadString(packet, offset));
                break;
            }

            case 5:
            {
                Slot slot = ReadSlot(packet, offset);
                meta_data.AddValue(slot);
                break;
            }

            case 6:
            {
                meta_data.AddValue(ReadVector<int>(packet, offset));
                break;
            }

            case 7:
            {
                meta_data.AddValue(ReadVector<float>(packet, offset));
                break;
            }

            default:
                std::cerr << "UNKNOWN METADATA TYPE: " << type << std::endl;;
            }

            index = ReadByte(packet, offset);
        }

        return meta_data;
    }

    Chunk PacketDecoder::ReadChunk(int x, int z, bool ground_up_continuous, bool sky_light_sent, uint16_t primary_bitmask, uint8_t* packet, size_t& offset)
    {
        Chunk chunk = Chunk(x, z, primary_bitmask);

        int i = 0;
        while (i < 15)
        {
            i++;
        }

        for (int section_num = 0; section_num < 16; section_num++)
        {
            if (((primary_bitmask >> section_num) & 1) == 0)
            {
                continue;
            }

            ChunkSection chunk_section;

            Buffer<uint16_t> block_ids = PacketDecoder::ReadShortArray(16 * 16 * 16, packet, offset);
            chunk_section.SetBlockIDs(block_ids);

            Buffer<uint8_t> emitted_light = PacketDecoder::ReadByteArray(16 * 16 * 16 / 2, packet, offset);
            chunk_section.SetEmittedLight(emitted_light);

            if (sky_light_sent)
            {
                Buffer<uint8_t> sky_light = PacketDecoder::ReadByteArray(16 * 16 * 16 / 2, packet, offset);
                chunk_section.SetSkyLight(sky_light);
            }

            if (ground_up_continuous)
            {
                Buffer<uint8_t> biome_index = PacketDecoder::ReadByteArray(16 * 16, packet, offset);
                chunk_section.SetBiomeIndex(biome_index);
            }

            chunk.AddSection(section_num, chunk_section);
        }

        return chunk;
    }

    Chunk PacketDecoder::ReadChunkBulk(Chunk& chunk, bool sky_light_sent, uint8_t* packet, size_t& offset)
    {
        std::list<ChunkSection> sections = std::list<ChunkSection>(chunk.GetNumSections());

        for (ChunkSection& section : sections)
        {
            Buffer<uint16_t> block_ids = PacketDecoder::ReadShortLittleEndianArray(16 * 16 * 16, packet, offset);
            section.SetBlockIDs(block_ids);
        }

        for (ChunkSection& section : sections)
        {
            Buffer<uint8_t> emitted_light = PacketDecoder::ReadByteArray(16 * 16 * 16 / 2, packet, offset);
            section.SetEmittedLight(emitted_light);
        }

        if (sky_light_sent)
        {
            for (ChunkSection& section : sections)
            {
                Buffer<uint8_t> sky_light = PacketDecoder::ReadByteArray(16 * 16 * 16 / 2, packet, offset);
                section.SetSkyLight(sky_light);
            }
        }

        Buffer<uint8_t> biome_index = PacketDecoder::ReadByteArray(16 * 16, packet, offset);
        chunk.SetBiomeIndex(biome_index.to_array<256>());

        int section_num = 0;

        std::list<int> section_nums;
        while (section_num < 16)
        {
            if (((chunk.GetPrimaryBitMask() >> section_num) & 1) != 0)
            {
                section_nums.push_back(section_num);
            }
            section_num++;
        }

        std::list<int>::iterator section_it = section_nums.begin();
        for (ChunkSection section : sections)
        {
            chunk.AddSection(*(section_it++), section);
        }

        return chunk;
    }



    //---- NBT ----//

    NBTList PacketDecoder::ReadNBTList(uint8_t* packet, size_t& offset)
    {
        NBTType list_type = (NBTType)ReadByte(packet, offset);
        NBTList nbt_list = NBTList(list_type);

        int32_t list_length = ReadInt(packet, offset);
        for (int i = 0; i < list_length; i++)
        {
            nbt_list.AddElement(ReadNBTTag(list_type, packet, offset, false));
        }

        return nbt_list;
    }

    std::string PacketDecoder::ReadNBTString(uint8_t* packet, size_t& offset)
    {
        uint16_t length = ReadShort(packet, offset);
        std::string str = "";
        for (size_t i = 0; i < length; i++)
        {
            str += packet[offset++];
        }
        return str;
    }

    NBTTag PacketDecoder::ReadNextNBTTag(uint8_t* packet, size_t& offset)
    {
        NBTType type = (NBTType)ReadByte(packet, offset);

        return ReadNBTTag(type, packet, offset);
    }

    NBTTag PacketDecoder::ReadNBTTag(NBTType type, uint8_t* packet, size_t& offset, bool has_name)
    {
        std::string name;
        if (type != NBTType::TAG_END && has_name)
        {
            uint16_t name_length = ReadShort(packet, offset);
            name = ReadString(name_length, packet, offset);
        }
        else
        {
            name = "NONE";
        }

        return ReadNBTTag(type, name, packet, offset);
    }

    NBTTag PacketDecoder::ReadNBTTag(NBTType type, std::string name, uint8_t* packet, size_t& offset)
    {
        switch (type)
        {
        case NBTType::TAG_BYTE:
            return NBTTag(type, name, (int8_t)ReadByte(packet, offset));
        case NBTType::TAG_SHORT:
            return NBTTag(type, name, (int16_t)ReadShort(packet, offset));
        case NBTType::TAG_INT:
            return NBTTag(type, name, (int32_t)ReadInt(packet, offset));
        case NBTType::TAG_LONG:
            return NBTTag(type, name, (int64_t)ReadLong(packet, offset));
        case NBTType::TAG_FLOAT:
            return NBTTag(type, name, (float)ReadFloat(packet, offset));
        case NBTType::TAG_DOUBLE:
            return NBTTag(type, name, (double)ReadDouble(packet, offset));
        case NBTType::TAG_STRING:
            return NBTTag(type, name, ReadNBTString(packet, offset));
        case NBTType::TAG_BYTE_ARRAY:
        {
            int16_t length = ReadInt(packet, offset);
            return NBTTag(type, name, ReadByteArray(length, packet, offset));
        }
        case NBTType::TAG_INT_ARRAY:
        {
            int16_t length = ReadInt(packet, offset);
            return NBTTag(type, name, ReadIntArray(length, packet, offset));
        }
        case NBTType::TAG_LONG_ARRAY:
        {
            int16_t length = ReadInt(packet, offset);
            return NBTTag(type, name, ReadLongArray(length, packet, offset));
        }
        case NBTType::TAG_LIST:
            return NBTTag(type, name, ReadNBTList(packet, offset));
        case NBTType::TAG_COMPOUND:
            return NBTTag(type, name, ReadNBTTagCompound(packet, offset));
        case NBTType::TAG_END:
            return NBTTag(NBTType::TAG_END, "NONE", 0);
        }

        return NBTTag(NBTType::UNKNOWN, name, -1);
    }

    NBTTagCompound PacketDecoder::ReadNBT(uint8_t* packet, size_t& offset)
    {
        return ReadNBTTagCompound(packet, offset, true);
    }

    NBTTagCompound PacketDecoder::ReadNBTTagCompound(uint8_t* packet, size_t& offset, bool parent)
    {
        NBTTagCompound tag_compound = NBTTagCompound();
        NBTTag tag;

        do
        {
            tag = ReadNextNBTTag(packet, offset);
            tag_compound.AddTag(tag);

            if (parent)
            {
                return tag_compound;
            }
        } while (tag.GetType() != NBTType::TAG_END);

        return tag_compound;
    }
}