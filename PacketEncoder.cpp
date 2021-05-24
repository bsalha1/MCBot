#include "PacketEncoder.h"
#include <cstdarg>

void mcbot::PacketEncoder::write_var_int(int value, uint8_t* packet, size_t& offset)
{
    do
    {
        char temp = (char)(value & 0b01111111);
        value >>= 7;
        if (value != 0)
        {
            temp |= 0b10000000;
        }
        packet[offset++] = temp;
    } while (value != 0);
}

int mcbot::PacketEncoder::get_var_int_size(int value)
{
    if (value == 0)
    {
        return 1;
    }
    return floor(log(value) / log(128)) + 1;
}

int mcbot::PacketEncoder::get_var_int_size(std::initializer_list<int> values)
{
    int size = 0;

    for (int value : values)
    {
        size += get_var_int_size(value);
    }

    return size;
}

void mcbot::PacketEncoder::write_byte_array(uint8_t* bytes, int bytes_length, uint8_t* packet, size_t& offset)
{
    for (int i = 0; i < bytes_length; i++)
    {
        write_byte(bytes[i], packet, offset);
    }
}

void mcbot::PacketEncoder::write_int_array(uint32_t* ints, int ints_length, uint8_t* packet, size_t& offset)
{
    for (int i = 0; i < ints_length; i++)
    {
        write_int(ints[i], packet, offset);
    }
}

void mcbot::PacketEncoder::write_long_array(uint64_t* longs, int longs_length, uint8_t* packet, size_t& offset)
{
    for (int i = 0; i < longs_length; i++)
    {
        write_long(longs[i], packet, offset);
    }
}

void mcbot::PacketEncoder::write_string(char* string, uint8_t* packet, size_t& offset)
{
    size_t string_length = strlen(string);
    write_var_int(string_length, packet, offset);

    for (int i = 0; i < string_length; i++)
    {
        packet[offset++] = string[i];
    }
}

void mcbot::PacketEncoder::write_string(std::string string, uint8_t* packet, size_t& offset)
{
    int length = string.length();
    write_var_int(length, packet, offset);

    for (int i = 0; i < length; i++)
    {
        packet[offset++] = string[i];
    }
}

void mcbot::PacketEncoder::write_nbt_string(std::string string, uint8_t* packet, size_t& offset)
{
    if (string == "NONE")
    {
        write_short(0, packet, offset);
        return;
    }

    int length = string.length();
    write_short(length, packet, offset);

    for (int i = 0; i < length; i++)
    {
        packet[offset++] = string[i];
    }
}

void mcbot::PacketEncoder::write_boolean(bool value, uint8_t* packet, size_t& offset)
{
    if (value)
    {
        packet[offset++] = 1;
    }
    else
    {
        packet[offset++] = 0;
    }
}

void mcbot::PacketEncoder::write_byte(uint8_t num, uint8_t* packet, size_t& offset)
{
    packet[offset++] = (num >> 0) & 0xFF;
}

void mcbot::PacketEncoder::write_short(uint16_t num, uint8_t* packet, size_t& offset)
{
    packet[offset++] = (num >> 8) & 0xFF;
    packet[offset++] = (num >> 0) & 0xFF;
}

void mcbot::PacketEncoder::write_int(uint32_t num, uint8_t* packet, size_t& offset)
{
    packet[offset++] = (num >> 24) & 0xFF;
    packet[offset++] = (num >> 16) & 0xFF;
    packet[offset++] = (num >> 8) & 0xFF;
    packet[offset++] = (num >> 0) & 0xFF;
}

void mcbot::PacketEncoder::write_long(uint64_t num, uint8_t* packet, size_t& offset)
{
    packet[offset++] = (num >> 56) & 0xFF;
    packet[offset++] = (num >> 48) & 0xFF;
    packet[offset++] = (num >> 40) & 0xFF;
    packet[offset++] = (num >> 32) & 0xFF;
    packet[offset++] = (num >> 24) & 0xFF;
    packet[offset++] = (num >> 16) & 0xFF;
    packet[offset++] = (num >> 8) & 0xFF;
    packet[offset++] = (num >> 0) & 0xFF;
}

void mcbot::PacketEncoder::write_double(double num, uint8_t* packet, size_t& offset)
{
    uint64_t bytes = *((uint64_t*)&num);
    write_long(bytes, packet, offset);
}

void mcbot::PacketEncoder::write_float(float num, uint8_t* packet, size_t& offset)
{
    uint32_t bytes = *((uint32_t*)&num);
    write_int(bytes, packet, offset);
}

void mcbot::PacketEncoder::write_position(int x, int y, int z, uint8_t* packet, size_t& offset)
{
    uint64_t bytes = (uint64_t) z & 0x3FFFFFF;
    bytes |= (uint64_t) y << 26;
    bytes |= (uint64_t)(x & 0x3FFFFFF) << 38;
    write_long(bytes, packet, offset);
}


void mcbot::PacketEncoder::write_slot(mcbot::Slot slot, uint8_t* packet, size_t& offset)
{
    slot.serialize(packet, offset);
}

void mcbot::PacketEncoder::write_nbt(mcbot::NBTTagCompound nbt, uint8_t* packet, size_t& offset)
{
    nbt.serialize(packet, offset);
}
