#include "PacketEncoder.h"

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

size_t mcbot::PacketEncoder::get_var_int_size(int value)
{
    if (value == 0)
    {
        return 1;
    }
    return floor(log(value) / log(128)) + 1;
}

void mcbot::PacketEncoder::write_byte_array(uint8_t* bytes, int bytes_length, uint8_t* packet, size_t& offset)
{
    for (int i = 0; i < bytes_length; i++)
    {
        packet[offset++] = bytes[i];
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
    packet[offset] = 0;
}

void mcbot::PacketEncoder::write_ushort(unsigned short num, uint8_t* packet, size_t& offset)
{
    packet[offset++] = num >> 8;
    packet[offset++] = num & 0xFF;
}
