#include "PacketDecoder.h"

int32_t mcbot::PacketDecoder::read_var_int(uint8_t* packet, size_t& offset)
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

int64_t mcbot::PacketDecoder::read_var_long(uint8_t* packet, size_t& offset)
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

uint32_t mcbot::PacketDecoder::read_int(uint8_t* packet, size_t& offset)
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

uint16_t mcbot::PacketDecoder::read_short(uint8_t* packet, size_t& offset)
{
    uint8_t byte2 = packet[offset++];
    uint8_t byte1 = packet[offset++];

    uint16_t result =
        byte2 << 8 |
        byte1 << 0;
    return result;
}

uint64_t mcbot::PacketDecoder::read_long(uint8_t* packet, size_t& offset)
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

uint8_t mcbot::PacketDecoder::read_byte(uint8_t* packet, size_t& offset)
{
    return packet[offset++];
}

uint8_t mcbot::PacketDecoder::peek_byte(uint8_t* packet, size_t offset)
{
    return packet[offset++];
}

float mcbot::PacketDecoder::read_float(uint8_t* packet, size_t& offset)
{
    uint32_t bytes = read_int(packet, offset);
    return *((float*)&bytes);
}

double mcbot::PacketDecoder::read_double(uint8_t* packet, size_t& offset)
{
    uint64_t bytes = read_long(packet, offset);
    return *((double*)&bytes);
}

bool mcbot::PacketDecoder::read_boolean(uint8_t* packet, size_t& offset)
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

std::string mcbot::PacketDecoder::read_string(uint8_t* packet, size_t& offset)
{
    int length = read_var_int(packet, offset);
    std::string string = "";
    for (size_t i = 0; i < length; i++)
    {
        string += packet[offset++];
    }
    return string;
}

std::string mcbot::PacketDecoder::read_string(int length, uint8_t* packet, size_t& offset)
{
    std::string string = "";
    for (size_t i = 0; i < length; i++)
    {
        string += packet[offset++];
    }
    return string;
}

mcbot::UUID mcbot::PacketDecoder::read_uuid(uint8_t* packet, size_t& offset)
{
    char bytes[16] = { 0 };
    for (int i = 0; i < 16; i++)
    {
        bytes[i] = packet[offset++];
    }
    return mcbot::UUID(bytes);
}

mcbot::Slot mcbot::PacketDecoder::read_slot(uint8_t* packet, size_t& offset)
{
    short item_id = read_short(packet, offset);
    if (item_id >= 0)
    {
        uint8_t item_count = read_byte(packet, offset);
        short data = read_short(packet, offset);
        mcbot::NBT nbt = read_nbt(packet, offset);
        return mcbot::Slot(item_id, item_count, data, nbt);
    }
    else
    {
        return mcbot::Slot();
    }
}

mcbot::Color mcbot::PacketDecoder::read_color(uint8_t* packet, size_t& offset)
{
    float r = read_float(packet, offset);
    float g = read_float(packet, offset);
    float b = read_float(packet, offset);
    float scale = read_float(packet, offset);

    return mcbot::Color(r, g, b, scale);
}

mcbot::Particle mcbot::PacketDecoder::read_particle(uint8_t* packet, size_t& offset)
{
    int id = read_var_int(packet, offset);
    switch (id)
    {
    case 3:
    {
        int block_state = read_var_int(packet, offset);
        return mcbot::Particle(id, block_state);
    }

    case 14:
    {
        mcbot::Color dust_color = read_color(packet, offset);
        return mcbot::Particle(id, dust_color);
    }

    case 23:
    {
        float block_state = read_var_int(packet, offset);
        return mcbot::Particle(id, block_state);
    }

    case 32:
    {
        mcbot::Slot slot = read_slot(packet, offset);
        return mcbot::Particle(id, slot);
    }
    }

    return mcbot::Particle(id);
}

void mcbot::PacketDecoder::read_byte_array(uint8_t* bytes, int length, uint8_t* packet, size_t& offset)
{
    for (int i = 0; i < length; i++)
    {
        bytes[i] = packet[offset++];
    }
}

mcbot::Buffer<char> mcbot::PacketDecoder::read_byte_array(int length, uint8_t* packet, size_t& offset)
{
    mcbot::Buffer<char> buffer = mcbot::Buffer<char>(length);
    for (int i = 0; i < length; i++)
    {
        buffer.put(packet[offset++]);
    }
    return buffer;
}

mcbot::Buffer<int> mcbot::PacketDecoder::read_int_array(int length, uint8_t* packet, size_t& offset)
{
    mcbot::Buffer<int> buffer = mcbot::Buffer<int>(length);
    for (int i = 0; i < length; i++)
    {
        buffer.put(read_int(packet, offset));
    }
    return buffer;
}

mcbot::Buffer<int> mcbot::PacketDecoder::read_var_int_array(int length, uint8_t* packet, size_t& offset)
{
    mcbot::Buffer<int> buffer = mcbot::Buffer<int>(length);
    for (int i = 0; i < length; i++)
    {
        buffer.put(read_var_int(packet, offset));
    }
    return buffer;
}

mcbot::Buffer<long> mcbot::PacketDecoder::read_long_array(int length, uint8_t* packet, size_t& offset)
{
    mcbot::Buffer<long> buffer = mcbot::Buffer<long>(length);
    for (int i = 0; i < length; i++)
    {
        buffer.put(read_long(packet, offset));
    }

    return buffer;
}


std::list<std::string> mcbot::PacketDecoder::read_string_array(int length, uint8_t* packet, size_t& offset)
{
    std::list<std::string> strings;

    for (int i = 0; i < length; i++)
    {
        strings.push_back(read_string(packet, offset));
    }

    return strings;
}

std::list<mcbot::Statistic> mcbot::PacketDecoder::read_statistic_array(int length, uint8_t* packet, size_t& offset)
{
    std::list<mcbot::Statistic> statistics;

    for (int i = 0; i < length; i++)
    {
        std::string name = read_string(packet, offset);
        int value = read_var_int(packet, offset);
        statistics.push_back(mcbot::Statistic(name, value));
    }

    return statistics;
}

std::list<mcbot::PlayerProperty> mcbot::PacketDecoder::read_property_array(int length, uint8_t* packet, size_t& offset)
{
    std::list<mcbot::PlayerProperty> properties;

    for (int i = 0; i < length; i++)
    {
        std::string name = read_string(packet, offset);
        std::string value = read_string(packet, offset);
        bool is_signed = read_boolean(packet, offset);
        std::string signature = is_signed ? read_string(packet, offset) : "";
        properties.push_back(mcbot::PlayerProperty(name, value, is_signed, signature));
    }

    return properties;
}

std::list<mcbot::Slot> mcbot::PacketDecoder::read_slot_array(int length, uint8_t* packet, size_t& offset)
{
    std::list<mcbot::Slot> slots;

    for (int i = 0; i < length; i++)
    {
        slots.push_back(read_slot(packet, offset));
    }

    return slots;
}

mcbot::Position mcbot::PacketDecoder::read_position(uint8_t* packet, size_t& offset)
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

    return mcbot::Position(x, y, z);
}

mcbot::VillagerData mcbot::PacketDecoder::read_villager_data(uint8_t* packet, size_t& offset)
{
    int type = read_var_int(packet, offset);
    int profession = read_var_int(packet, offset);
    int level = read_var_int(packet, offset);
    return mcbot::VillagerData(type, profession, level);
}

mcbot::AttributeModifier mcbot::PacketDecoder::read_attribute_modifier(uint8_t* packet, size_t& offset)
{
    mcbot::UUID uuid = read_uuid(packet, offset);
    double amount = read_double(packet, offset);
    uint8_t operation = read_byte(packet, offset);
    return mcbot::AttributeModifier(uuid, amount, operation);
}

mcbot::Attribute mcbot::PacketDecoder::read_attribute(uint8_t* packet, size_t& offset)
{
    std::string key = read_string(packet, offset);
    double value = read_double(packet, offset);
    int num_modifiers = read_var_int(packet, offset);

    std::list<mcbot::AttributeModifier> modifiers;
    for (int i = 0; i < num_modifiers; i++)
    {
        modifiers.push_back(read_attribute_modifier(packet, offset));
    }
    return mcbot::Attribute(key, value, modifiers);
}

std::list<mcbot::NBT> mcbot::PacketDecoder::read_nbt_list(int32_t length, mcbot::NBTType list_type, uint8_t* packet, size_t& offset)
{
    std::list<mcbot::NBT> nbt_list;

    for (int i = 0; i < length; i++)
    {
        nbt_list.push_back(read_nbt_part(packet, offset, false, true, list_type));
    }

    return nbt_list;
}

mcbot::NBT mcbot::PacketDecoder::read_nbt_part(uint8_t* packet, size_t& offset, bool parent, bool list, mcbot::NBTType list_type)
{
    mcbot::NBTType type;
    mcbot::NBT nbt;

    if (list)
    {
        type = list_type;
    }

    do
    {
        // Lists have no names and contain all the same types 
        std::string name;
        if (!list)
        {
            type = (mcbot::NBTType) read_byte(packet, offset);

            if (type == mcbot::NBTType::TAG_END)
            {
                return nbt;
            }

            uint16_t name_length = read_short(packet, offset);
            name = read_string(name_length, packet, offset);
        }
        else
        {
            name = "NONE";
        }

        switch (type)
        {

        case mcbot::NBTType::TAG_END:
        {
            return nbt;
        }

        case mcbot::NBTType::TAG_BYTE:
        {
            nbt.add_byte(name, read_byte(packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_SHORT:
        {
            nbt.add_short(name, read_short(packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_INT:
        {
            nbt.add_int(name, read_int(packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_LONG:
        {
            nbt.add_long(name, read_long(packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_FLOAT:
        {
            nbt.add_float(name, read_float(packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_DOUBLE:
        {
            nbt.add_double(name, read_double(packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_BYTE_ARRAY:
        {
            int32_t length = read_int(packet, offset);
            nbt.add_byte_array(name, read_byte_array(length, packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_INT_ARRAY:
        {
            int32_t length = read_int(packet, offset);
            nbt.add_int_array(name, read_int_array(length, packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_LONG_ARRAY:
        {
            int32_t length = read_int(packet, offset);
            nbt.add_long_array(name, read_long_array(length, packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_STRING:
        {
            uint16_t length = read_short(packet, offset);
            nbt.add_string(name, read_string(length, packet, offset));
            break;
        }

        case mcbot::NBTType::TAG_LIST:
        {
            mcbot::NBTType list_type = (mcbot::NBTType) read_byte(packet, offset);
            int32_t length = read_int(packet, offset);
            nbt.add_nbt_list(name, read_nbt_list(length, list_type, packet, offset));
            break;
        }
        case mcbot::NBTType::TAG_COMPOUND:
        {
            nbt.add_nbt(name, read_nbt_part(packet, offset, false));
            break;
        }
        }

        if (parent || list)
        {
            return nbt;
        }

    } while (true);

    return nbt;
}

mcbot::NBT mcbot::PacketDecoder::read_nbt(uint8_t* packet, size_t& offset)
{
    return read_nbt_part(packet, offset, true);;
}

mcbot::EntityMetaData mcbot::PacketDecoder::read_meta_data(uint8_t* packet, size_t& offset)
{
    mcbot::EntityMetaData meta_data;

    uint8_t index = read_byte(packet, offset);
    while (index != 0x7F)
    {
        int a = index & 0x1F;
        int type = index >> 5;

        switch (type)
        {
        case 0:
        {
            meta_data.add_value(read_byte(packet, offset));
            break;
        }

        case 1:
        {
            meta_data.add_value(read_short(packet, offset));
            break;
        }

        case 2:
        {
            meta_data.add_value(read_int(packet, offset));
            break;
        }

        case 3:
        {
            meta_data.add_value(read_float(packet, offset));
            break;
        }

        case 4:
        {
            meta_data.add_value(read_string(packet, offset));
            break;
        }

        case 5:
        {
            mcbot::Slot slot = read_slot(packet, offset);
            meta_data.add_value(slot);
            break;
        }

        case 6:
        {
            meta_data.add_value(read_vector<int>(packet, offset));
            break;
        }

        case 7:
        {
            meta_data.add_value(read_vector<float>(packet, offset));
            break;
        }

        default:
            std::cerr << "UNKNOWN METADATA TYPE: " << type << std::endl;;
        }

        index = read_byte(packet, offset);
    }

    return meta_data;
}
