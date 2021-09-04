#include "PacketEncoder.h"
#include <cstdarg>

namespace McBot
{
    //---- VarInt ----//

    void PacketEncoder::WriteVarInt(int value, uint8_t* data, size_t& offset)
    {
        do
        {
            char temp = (char)(value & 0b01111111);
            value >>= 7;
            if (value != 0)
            {
                temp |= 0b10000000;
            }
            data[offset++] = temp;
        } while (value != 0);
    }

    void PacketEncoder::WriteVarInt(int value, Packet& packet)
    {
        do
        {
            char temp = (char)(value & 0b01111111);
            value >>= 7;
            if (value != 0)
            {
                temp |= 0b10000000;
            }
            packet.AddByte(temp);
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

        for (auto value : values)
        {
            size += GetVarIntNumBytes(value);
        }

        return size;
    }



    //---- Arrays ----//

    void PacketEncoder::WriteByteArray(uint8_t* bytes, int bytes_length, Packet& packet)
    {
        for (int i = 0; i < bytes_length; i++)
        {
            WriteByte(bytes[i], packet);
        }
    }

    void PacketEncoder::WriteIntArray(uint32_t* ints, int ints_length, Packet& packet)
    {
        for (int i = 0; i < ints_length; i++)
        {
            WriteInt(ints[i], packet);
        }
    }

    void PacketEncoder::WriteLongArray(uint64_t* longs, int longs_length, Packet& packet)
    {
        for (int i = 0; i < longs_length; i++)
        {
            WriteLong(longs[i], packet);
        }
    }



    //---- Strings ----//

    void PacketEncoder::WriteString(char* string, Packet& packet)
    {
        size_t string_length = strlen(string);
        WriteVarInt(string_length, packet);

        for (int i = 0; i < string_length; i++)
        {
            packet.AddByte(string[i]);
        }
    }

    void PacketEncoder::WriteString(std::string string, Packet& packet)
    {
        int length = string.length();
        WriteVarInt(length, packet);

        for (int i = 0; i < length; i++)
        {
            packet.AddByte(string[i]);
        }
    }

    void PacketEncoder::WriteNBTString(std::string string, Packet& packet)
    {
        if (string == "NONE")
        {
            WriteShort(0, packet);
            return;
        }

        int length = string.length();
        WriteShort(length, packet);

        for (int i = 0; i < length; i++)
        {
            packet.AddByte(string[i]);
        }
    }

    int PacketEncoder::GetStringNumBytes(std::string string)
    {
        auto len = string.length();
        return len + GetVarIntNumBytes(len);
    }

    int PacketEncoder::GetStringNumBytes(std::initializer_list<std::string> values)
    {
        int size = 0;

        for (auto value : values)
        {
            size += GetStringNumBytes(value);
        }

        return size;
    }




    //---- Basic Types ----//

    void PacketEncoder::WriteBoolean(bool value, Packet& packet)
    {
        if (value)
        {
            packet.AddByte(1);
        }
        else
        {
            packet.AddByte(0);
        }
    }

    void PacketEncoder::WriteByte(uint8_t num, Packet& packet)
    {
        packet.AddByte(num);
    }

    void PacketEncoder::WriteShort(uint16_t num, Packet& packet)
    {
        packet.AddByte((num >> 8) & 0xFF);
        packet.AddByte((num >> 0) & 0xFF);
    }

    void PacketEncoder::WriteInt(uint32_t num, Packet& packet)
    {
        packet.AddByte((num >> 24) & 0xFF);
        packet.AddByte((num >> 16) & 0xFF);
        packet.AddByte((num >> 8) & 0xFF);
        packet.AddByte((num >> 0) & 0xFF);
    }

    void PacketEncoder::WriteLong(uint64_t num, Packet& packet)
    {
        packet.AddByte((num >> 56) & 0xFF);
        packet.AddByte((num >> 48) & 0xFF);
        packet.AddByte((num >> 40) & 0xFF);
        packet.AddByte((num >> 32) & 0xFF);
        packet.AddByte((num >> 24) & 0xFF);
        packet.AddByte((num >> 16) & 0xFF);
        packet.AddByte((num >> 8) & 0xFF);
        packet.AddByte((num >> 0) & 0xFF);
    }

    void PacketEncoder::WriteDouble(double num, Packet& packet)
    {
        uint64_t bytes = *((uint64_t*)&num);
        WriteLong(bytes, packet);
    }

    void PacketEncoder::WriteFloat(float num, Packet& packet)
    {
        uint32_t bytes = *((uint32_t*)&num);
        WriteInt(bytes, packet);
    }



    //---- Classes ----//

    void PacketEncoder::WritePosition(int x, int y, int z, Packet& packet)
    {
        uint64_t bytes = (uint64_t)z & 0x3FFFFFF;
        bytes |= (uint64_t)y << 26;
        bytes |= (uint64_t)(x & 0x3FFFFFF) << 38;
        WriteLong(bytes, packet);
    }
}