#include "PacketDecoder.h"
#include "StringUtils.h"

namespace mcbot
{
    //---- Basic Types ----//
    int32_t PacketDecoder::ReadVarInt(Packet& packet)
    {
        int num_read = 0;
        int result = 0;
        uint8_t read;
        do {
            read = packet.data[packet.offset + num_read];
            int value = (read & 0b01111111);
            result |= (value << (7 * num_read));

            num_read++;
            if (num_read > 5)
            {
                packet.offset += num_read;
                fprintf(stderr, "VarInt out of bounds");
                return -1;
            }
        } while ((read & 0b10000000) != 0);

        packet.offset += num_read;
        return result;
    }

    int64_t PacketDecoder::ReadVarLong(Packet& packet)
    {
        int num_read = 0;
        long result = 0;
        uint8_t read;
        do {
            read = packet.data[packet.offset + num_read];
            long value = (read & 0b01111111);
            result |= (value << (7 * num_read));

            num_read++;
            if (num_read > 10)
            {
                packet.offset += num_read;
                fprintf(stderr, "VarLong out of bounds");
            }
        } while ((read & 0b10000000) != 0);

        return result;
    }

    uint32_t PacketDecoder::ReadInt(Packet& packet)
    {
        uint8_t byte4 = packet.data[packet.offset++];
        uint8_t byte3 = packet.data[packet.offset++];
        uint8_t byte2 = packet.data[packet.offset++];
        uint8_t byte1 = packet.data[packet.offset++];

        uint32_t result =
            byte4 << 24 |
            byte3 << 16 |
            byte2 << 8 |
            byte1 << 0;
        return result;
    }

    uint16_t PacketDecoder::ReadShort(Packet& packet)
    {
        uint8_t byte2 = packet.data[packet.offset++];
        uint8_t byte1 = packet.data[packet.offset++];

        uint16_t result =
            byte2 << 8 |
            byte1 << 0;
        return result;
    }

    uint16_t PacketDecoder::ReadShortLittleEndian(Packet& packet)
    {
        uint8_t byte2 = packet.data[packet.offset++];
        uint8_t byte1 = packet.data[packet.offset++];

        uint16_t result =
            byte2 << 0 |
            byte1 << 8;
        return result;
    }

    uint64_t PacketDecoder::ReadLong(Packet& packet)
    {
        uint64_t byte8 = packet.data[packet.offset++];
        uint64_t byte7 = packet.data[packet.offset++];
        uint64_t byte6 = packet.data[packet.offset++];
        uint64_t byte5 = packet.data[packet.offset++];
        uint64_t byte4 = packet.data[packet.offset++];
        uint64_t byte3 = packet.data[packet.offset++];
        uint64_t byte2 = packet.data[packet.offset++];
        uint64_t byte1 = packet.data[packet.offset++];

        uint64_t result =
            byte8 << 56 |
            byte7 << 48 |
            byte6 << 40 |
            byte5 << 32 |
            byte4 << 24 |
            byte3 << 16 |
            byte2 << 8 |
            byte1 << 0;
        return result;
    }

    uint8_t PacketDecoder::ReadByte(Packet& packet)
    {
        return packet.data[packet.offset++];
    }

    uint8_t PacketDecoder::PeekByte(Packet& packet)
    {
        return packet.data[packet.offset++];
    }

    float PacketDecoder::ReadFloat(Packet& packet)
    {
        uint32_t bytes = ReadInt(packet);
        return *((float*)&bytes);
    }

    double PacketDecoder::ReadDouble(Packet& packet)
    {
        uint64_t bytes = ReadLong(packet);
        return *((double*)&bytes);
    }

    bool PacketDecoder::ReadBoolean(Packet& packet)
    {
        uint8_t value = packet.data[packet.offset++];
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

    std::string PacketDecoder::ReadString(Packet& packet)
    {
        int length = ReadVarInt(packet);
        std::string string = "";
        for (size_t i = 0; i < length; i++)
        {
            string += packet.data[packet.offset++];
        }
        return string;
    }

    std::string PacketDecoder::ReadString(int length, Packet& packet)
    {
        std::string string = "";
        for (size_t i = 0; i < length; i++)
        {
            string += packet.data[packet.offset++];
        }
        return string;
    }



    //---- Arrays ----//

    void PacketDecoder::ReadByteArray(uint8_t* bytes, int length, Packet& packet)
    {
        for (int i = 0; i < length; i++)
        {
            bytes[i] = packet.data[packet.offset++];
        }
    }

    Buffer<uint8_t> PacketDecoder::ReadByteArray(size_t length, Packet& packet)
    {
        Buffer<uint8_t> buffer = Buffer<uint8_t>(length);
        for (size_t i = 0; i < length; i++)
        {
            buffer.put(packet.data[packet.offset++]);
        }
        return buffer;
    }

    Buffer<uint16_t> PacketDecoder::ReadShortArray(int length, Packet& packet)
    {
        Buffer<uint16_t> buffer = Buffer<uint16_t>(length);
        for (int i = 0; i < length; i++)
        {
            buffer.put(ReadShort(packet));
        }
        return buffer;

    }

    Buffer<uint16_t> PacketDecoder::ReadShortLittleEndianArray(int length, Packet& packet)
    {
        Buffer<uint16_t> buffer = Buffer<uint16_t>(length);
        for (int i = 0; i < length; i++)
        {
            buffer.put(ReadShortLittleEndian(packet));
        }
        return buffer;

    }

    Buffer<int> PacketDecoder::ReadIntArray(int length, Packet& packet)
    {
        Buffer<int> buffer = Buffer<int>(length);
        for (int i = 0; i < length; i++)
        {
            buffer.put(ReadInt(packet));
        }
        return buffer;
    }

    Buffer<int> PacketDecoder::ReadVarIntArray(int length, Packet& packet)
    {
        Buffer<int> buffer = Buffer<int>(length);
        for (int i = 0; i < length; i++)
        {
            buffer.put(ReadVarInt(packet));
        }
        return buffer;
    }

    Buffer<long> PacketDecoder::ReadLongArray(int length, Packet& packet)
    {
        Buffer<long> buffer = Buffer<long>(length);
        for (int i = 0; i < length; i++)
        {
            buffer.put(ReadLong(packet));
        }

        return buffer;
    }


    std::list<std::string> PacketDecoder::ReadStringArray(int length, Packet& packet)
    {
        std::list<std::string> strings;

        for (int i = 0; i < length; i++)
        {
            strings.push_back(ReadString(packet));
        }

        return strings;
    }

    std::list<Statistic> PacketDecoder::ReadStatisticArray(int length, Packet& packet)
    {
        std::list<Statistic> statistics;

        for (int i = 0; i < length; i++)
        {
            std::string name = ReadString(packet);
            int value = ReadVarInt(packet);
            statistics.push_back(Statistic(name, value));
        }

        return statistics;
    }

    std::list<PlayerProperty> PacketDecoder::ReadPropertyArray(int length, Packet& packet)
    {
        std::list<PlayerProperty> properties;

        for (int i = 0; i < length; i++)
        {
            std::string name = ReadString(packet);
            std::string value = ReadString(packet);
            bool is_signed = ReadBoolean(packet);
            std::string signature = is_signed ? ReadString(packet) : "";
            properties.push_back(PlayerProperty(name, value, is_signed, signature));
        }

        return properties;
    }

    std::list<Slot> PacketDecoder::ReadSlotArray(int length, Packet& packet)
    {
        std::list<Slot> slots;

        for (int i = 0; i < length; i++)
        {
            slots.push_back(ReadSlot(packet));
        }

        return slots;
    }



    //---- Classes ----//

    UUID PacketDecoder::ReadUUID(Packet& packet)
    {
        char bytes[16] = { 0 };
        for (int i = 0; i < 16; i++)
        {
            bytes[i] = packet.data[packet.offset++];
        }
        return UUID(bytes);
    }

    Slot PacketDecoder::ReadSlot(Packet& packet)
    {
        short item_id = ReadShort(packet);
        if (item_id >= 0)
        {
            uint8_t item_count = ReadByte(packet);
            short data = ReadShort(packet);
            NBTTagCompound nbt = ReadNBT(packet);
            return Slot(item_id, item_count, data, nbt);
        }
        else
        {
            return Slot();
        }
    }

    Color PacketDecoder::ReadColor(Packet& packet)
    {
        float r = ReadFloat(packet);
        float g = ReadFloat(packet);
        float b = ReadFloat(packet);
        float scale = ReadFloat(packet);

        return Color(r, g, b, scale);
    }

    Particle PacketDecoder::ReadParticle(Packet& packet)
    {
        int id = ReadVarInt(packet);
        switch (id)
        {
        case 3:
        {
            int block_state = ReadVarInt(packet);
            return Particle(id, block_state);
        }

        case 14:
        {
            Color dust_color = ReadColor(packet);
            return Particle(id, dust_color);
        }

        case 23:
        {
            int block_state = ReadVarInt(packet);
            return Particle(id, block_state);
        }

        case 32:
        {
            Slot slot = ReadSlot(packet);
            return Particle(id, slot);
        }
        }

        return Particle(id);
    }

    Position PacketDecoder::ReadPosition(Packet& packet)
    {
        // Parse X Coordinate (26 bit signed integer)
        uint8_t byte4 = packet.data[packet.offset++];
        uint8_t byte3 = packet.data[packet.offset++];
        uint8_t byte2 = packet.data[packet.offset++];
        uint8_t byte1 = packet.data[packet.offset] & 0xC0; // xx00 0000

        uint32_t x_bits = ((byte4 << 24) | (byte3 << 16) | (byte2 << 8) | (byte1 << 0)) >> 6;
        int32_t x = x_bits & (1 << 25) ? x_bits - (0x3FFFFFF + 1) : x_bits;

        // Parse Y Coordinate (12 bit signed integer)
        byte2 = packet.data[packet.offset++] & 0x3F;
        byte1 = packet.data[packet.offset] & 0xFC;

        uint16_t y_bits = ((byte2 << 8) | (byte1 << 0)) >> 2;
        int16_t y = y_bits & (1 << 11) ? y_bits - (0xFFF + 1) : y_bits;

        // Parse Z Coordinate (26 bit signed integer)
        byte4 = packet.data[packet.offset++] & 0x03;
        byte3 = packet.data[packet.offset++];
        byte2 = packet.data[packet.offset++];
        byte1 = packet.data[packet.offset++];

        uint32_t z_bits = (byte4 << 24) | (byte3 << 16) | (byte2 << 8) | (byte1 << 0);
        int32_t z = z_bits & (1 << 25) ? z_bits - (0x3FFFFFF + 1) : z_bits;

        return Position(x, y, z);
    }

    VillagerData PacketDecoder::ReadVillagerData(Packet& packet)
    {
        int type = ReadVarInt(packet);
        int profession = ReadVarInt(packet);
        int level = ReadVarInt(packet);
        return VillagerData(type, profession, level);
    }

    AttributeModifier PacketDecoder::ReadAttributeModifier(Packet& packet)
    {
        UUID uuid = ReadUUID(packet);
        double amount = ReadDouble(packet);
        uint8_t operation = ReadByte(packet);
        return AttributeModifier(uuid, amount, operation);
    }

    Attribute PacketDecoder::ReadAttribute(Packet& packet)
    {
        std::string key = ReadString(packet);
        double value = ReadDouble(packet);
        int num_modifiers = ReadVarInt(packet);

        std::list<AttributeModifier> modifiers;
        for (int i = 0; i < num_modifiers; i++)
        {
            modifiers.push_back(ReadAttributeModifier(packet));
        }
        return Attribute(key, value, modifiers);
    }

    EntityMetaData PacketDecoder::ReadMetaData(Packet& packet)
    {
        EntityMetaData meta_data;

        uint8_t index = ReadByte(packet);
        while (index != 0x7F)
        {
            int a = index & 0x1F;
            int type = index >> 5;

            switch (type)
            {
            case 0:
            {
                meta_data.AddValue(ReadByte(packet));
                break;
            }

            case 1:
            {
                meta_data.AddValue(ReadShort(packet));
                break;
            }

            case 2:
            {
                meta_data.AddValue(ReadInt(packet));
                break;
            }

            case 3:
            {
                meta_data.AddValue(ReadFloat(packet));
                break;
            }

            case 4:
            {
                meta_data.AddValue(ReadString(packet));
                break;
            }

            case 5:
            {
                Slot slot = ReadSlot(packet);
                meta_data.AddValue(slot);
                break;
            }

            case 6:
            {
                meta_data.AddValue(ReadVector<int>(packet));
                break;
            }

            case 7:
            {
                meta_data.AddValue(ReadVector<float>(packet));
                break;
            }

            default:
                std::cerr << "UNKNOWN METADATA TYPE: " << type << std::endl;;
            }

            index = ReadByte(packet);
        }

        return meta_data;
    }

    Chunk PacketDecoder::ReadChunk(int x, int z, bool ground_up_continuous, bool sky_light_sent, uint16_t primary_bitmask, Packet& packet)
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

            Buffer<uint16_t> block_ids = PacketDecoder::ReadShortArray(16 * 16 * 16, packet);
            chunk_section.SetBlockIDs(block_ids);

            Buffer<uint8_t> emitted_light = PacketDecoder::ReadByteArray(16 * 16 * 16 / 2, packet);
            chunk_section.SetEmittedLight(emitted_light);

            if (sky_light_sent)
            {
                Buffer<uint8_t> sky_light = PacketDecoder::ReadByteArray(16 * 16 * 16 / 2, packet);
                chunk_section.SetSkyLight(sky_light);
            }

            if (ground_up_continuous)
            {
                Buffer<uint8_t> biome_index = PacketDecoder::ReadByteArray(16 * 16, packet);
                chunk_section.SetBiomeIndex(biome_index);
            }

            chunk.AddSection(section_num, chunk_section);
        }

        return chunk;
    }

    Chunk PacketDecoder::ReadChunkBulk(Chunk& chunk, bool sky_light_sent, Packet& packet)
    {
        std::list<ChunkSection> sections = std::list<ChunkSection>(chunk.GetNumSections());

        for (ChunkSection& section : sections)
        {
            Buffer<uint16_t> block_ids = PacketDecoder::ReadShortLittleEndianArray(16 * 16 * 16, packet);
            section.SetBlockIDs(block_ids);
        }

        for (ChunkSection& section : sections)
        {
            Buffer<uint8_t> emitted_light = PacketDecoder::ReadByteArray(16 * 16 * 16 / 2, packet);
            section.SetEmittedLight(emitted_light);
        }

        if (sky_light_sent)
        {
            for (ChunkSection& section : sections)
            {
                Buffer<uint8_t> sky_light = PacketDecoder::ReadByteArray(16 * 16 * 16 / 2, packet);
                section.SetSkyLight(sky_light);
            }
        }

        Buffer<uint8_t> biome_index = PacketDecoder::ReadByteArray(16 * 16, packet);
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

    NBTList PacketDecoder::ReadNBTList(Packet& packet)
    {
        NBTType list_type = (NBTType)ReadByte(packet);
        NBTList nbt_list = NBTList(list_type);

        int32_t list_length = ReadInt(packet);
        for (int i = 0; i < list_length; i++)
        {
            nbt_list.AddElement(ReadNBTTag(list_type, packet, false));
        }

        return nbt_list;
    }

    std::string PacketDecoder::ReadNBTString(Packet& packet)
    {
        uint16_t length = ReadShort(packet);
        std::string str = "";
        for (size_t i = 0; i < length; i++)
        {
            str += packet.data[packet.offset++];
        }
        return str;
    }

    NBTTag PacketDecoder::ReadNextNBTTag(Packet& packet)
    {
        NBTType type = (NBTType)ReadByte(packet);

        return ReadNBTTag(type, packet);
    }

    NBTTag PacketDecoder::ReadNBTTag(NBTType type, Packet& packet, bool has_name)
    {
        std::string name;
        if (type != NBTType::TAG_END && has_name)
        {
            uint16_t name_length = ReadShort(packet);
            name = ReadString(name_length, packet);
        }
        else
        {
            name = "NONE";
        }

        return ReadNBTTag(type, name, packet);
    }

    NBTTag PacketDecoder::ReadNBTTag(NBTType type, std::string name, Packet& packet)
    {
        switch (type)
        {
        case NBTType::TAG_BYTE:
            return NBTTag(type, name, (int8_t)ReadByte(packet));
        case NBTType::TAG_SHORT:
            return NBTTag(type, name, (int16_t)ReadShort(packet));
        case NBTType::TAG_INT:
            return NBTTag(type, name, (int32_t)ReadInt(packet));
        case NBTType::TAG_LONG:
            return NBTTag(type, name, (int64_t)ReadLong(packet));
        case NBTType::TAG_FLOAT:
            return NBTTag(type, name, (float)ReadFloat(packet));
        case NBTType::TAG_DOUBLE:
            return NBTTag(type, name, (double)ReadDouble(packet));
        case NBTType::TAG_STRING:
            return NBTTag(type, name, ReadNBTString(packet));
        case NBTType::TAG_BYTE_ARRAY:
        {
            int16_t length = ReadInt(packet);
            return NBTTag(type, name, ReadByteArray(length, packet));
        }
        case NBTType::TAG_INT_ARRAY:
        {
            int16_t length = ReadInt(packet);
            return NBTTag(type, name, ReadIntArray(length, packet));
        }
        case NBTType::TAG_LONG_ARRAY:
        {
            int16_t length = ReadInt(packet);
            return NBTTag(type, name, ReadLongArray(length, packet));
        }
        case NBTType::TAG_LIST:
            return NBTTag(type, name, ReadNBTList(packet));
        case NBTType::TAG_COMPOUND:
            return NBTTag(type, name, ReadNBTTagCompound(packet));
        case NBTType::TAG_END:
            return NBTTag(NBTType::TAG_END, "NONE", 0);
        }

        return NBTTag(NBTType::UNKNOWN, name, -1);
    }

    NBTTagCompound PacketDecoder::ReadNBT(Packet& packet)
    {
        return ReadNBTTagCompound(packet, true);
    }

    NBTTagCompound PacketDecoder::ReadNBTTagCompound(Packet& packet, bool parent)
    {
        NBTTagCompound tag_compound = NBTTagCompound();
        NBTTag tag;

        do
        {
            tag = ReadNextNBTTag(packet);
            tag_compound.AddTag(tag);

            if (parent)
            {
                return tag_compound;
            }
        } while (tag.GetType() != NBTType::TAG_END);

        return tag_compound;
    }
}