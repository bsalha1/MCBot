#include "PacketEncoder.h"
#include <cstdarg>

namespace mcbot
{
    //---- VarInt ----//

    void PacketEncoder::WriteVarInt(int value, uint8_t* packet, size_t& offset)
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

    int PacketEncoder::GetVarIntNumBytes(int value)
    {
        if (value == 0)
        {
            return 1;
        }
        return floor(log(value) / log(128)) + 1;
    }

    int PacketEncoder::GetVarIntNumBytes(std::initializer_list<int> values)
    {
        int size = 0;

        for (int value : values)
        {
            size += GetVarIntNumBytes(value);
        }

        return size;
    }



    //---- Arrays ----//

    void PacketEncoder::WriteByteArray(uint8_t* bytes, int bytes_length, uint8_t* packet, size_t& offset)
    {
        for (int i = 0; i < bytes_length; i++)
        {
            WriteByte(bytes[i], packet, offset);
        }
    }

    void PacketEncoder::WriteIntArray(uint32_t* ints, int ints_length, uint8_t* packet, size_t& offset)
    {
        for (int i = 0; i < ints_length; i++)
        {
            WriteInt(ints[i], packet, offset);
        }
    }

    void PacketEncoder::WriteLongArray(uint64_t* longs, int longs_length, uint8_t* packet, size_t& offset)
    {
        for (int i = 0; i < longs_length; i++)
        {
            WriteLong(longs[i], packet, offset);
        }
    }



    //---- Strings ----//

    void PacketEncoder::WriteString(char* string, uint8_t* packet, size_t& offset)
    {
        size_t string_length = strlen(string);
        WriteVarInt(string_length, packet, offset);

        for (int i = 0; i < string_length; i++)
        {
            packet[offset++] = string[i];
        }
    }

    void PacketEncoder::WriteString(std::string string, uint8_t* packet, size_t& offset)
    {
        int length = string.length();
        WriteVarInt(length, packet, offset);

        for (int i = 0; i < length; i++)
        {
            packet[offset++] = string[i];
        }
    }

    void PacketEncoder::WriteNBTString(std::string string, uint8_t* packet, size_t& offset)
    {
        if (string == "NONE")
        {
            WriteShort(0, packet, offset);
            return;
        }

        int length = string.length();
        WriteShort(length, packet, offset);

        for (int i = 0; i < length; i++)
        {
            packet[offset++] = string[i];
        }
    }



    //---- Basic Types ----//

    void PacketEncoder::WriteBoolean(bool value, uint8_t* packet, size_t& offset)
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

    void PacketEncoder::WriteByte(uint8_t num, uint8_t* packet, size_t& offset)
    {
        packet[offset++] = (num >> 0) & 0xFF;
    }

    void PacketEncoder::WriteShort(uint16_t num, uint8_t* packet, size_t& offset)
    {
        packet[offset++] = (num >> 8) & 0xFF;
        packet[offset++] = (num >> 0) & 0xFF;
    }

    void PacketEncoder::WriteInt(uint32_t num, uint8_t* packet, size_t& offset)
    {
        packet[offset++] = (num >> 24) & 0xFF;
        packet[offset++] = (num >> 16) & 0xFF;
        packet[offset++] = (num >> 8) & 0xFF;
        packet[offset++] = (num >> 0) & 0xFF;
    }

    void PacketEncoder::WriteLong(uint64_t num, uint8_t* packet, size_t& offset)
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

    void PacketEncoder::WriteDouble(double num, uint8_t* packet, size_t& offset)
    {
        uint64_t bytes = *((uint64_t*)&num);
        WriteLong(bytes, packet, offset);
    }

    void PacketEncoder::WriteFloat(float num, uint8_t* packet, size_t& offset)
    {
        uint32_t bytes = *((uint32_t*)&num);
        WriteInt(bytes, packet, offset);
    }



    //---- Classes ----//

    void PacketEncoder::WritePosition(int x, int y, int z, uint8_t* packet, size_t& offset)
    {
        uint64_t bytes = (uint64_t)z & 0x3FFFFFF;
        bytes |= (uint64_t)y << 26;
        bytes |= (uint64_t)(x & 0x3FFFFFF) << 38;
        WriteLong(bytes, packet, offset);
    }

    void PacketEncoder::WriteSerializable(Serializable const& serializable, uint8_t* packet, size_t& offset)
    {
        serializable.Serialize(packet, offset);
    }
}