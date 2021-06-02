#include "Socket.h"
#include "zlib.h"
#include "PacketEncoder.h"

namespace mcbot
{
    std::string get_openssl_error_string()
    {
        BIO* bio = BIO_new(BIO_s_mem());
        ERR_print_errors(bio);
        char* buf;
        size_t len = BIO_get_mem_data(bio, &buf);
        std::string ret(buf, len);
        BIO_free(bio);
        return ret;
    }

    int read_var_int(uint8_t* packet)
    {
        int num_read = 0;
        int result = 0;
        char read;
        int i = 0;
        do {
            read = packet[i + num_read];
            int value = (read & 0b01111111);
            result |= (value << (7 * num_read));

            num_read++;
            if (num_read > 5)
            {
                i += num_read;
                fprintf(stderr, "VarInt out of bounds");
                return -1;
            }
        } while ((read & 0b10000000) != 0);

        i += num_read;
        return result;
    }

    Socket::Socket(SOCKET socket)
    {
        this->socket = socket;
        this->encryption_enabled = false;
        this->compression_enabled = false;
        this->encrypt_ctx = NULL;
        this->decrypt_ctx = NULL;
        this->iv = NULL;
        this->key = NULL;
        this->max_uncompressed_length = -1;
    }

    Socket::Socket()
    {
        this->socket = -1;
        this->encryption_enabled = false;
        this->compression_enabled = false;
        this->encrypt_ctx = NULL;
        this->decrypt_ctx = NULL;
        this->iv = NULL;
        this->key = NULL;
        this->max_uncompressed_length = -1;
    }

    void Socket::InitEncryption(uint8_t* key, uint8_t* iv)
    {
        this->key = key;
        this->iv = iv;

        // Initialize encrypt context
        if (!(encrypt_ctx = EVP_CIPHER_CTX_new()))
        {
            std::cerr << "EVP_CIPHER_CTX_new error" << std::endl;
        }

        // Initialize encryptor
        if (1 != EVP_EncryptInit_ex(encrypt_ctx, EVP_aes_128_cfb8(), NULL, key, iv))
        {
            std::cerr << "EVP_EncryptInit_ex error" << std::endl;
        }

        // Initialize decrypt context
        if (!(decrypt_ctx = EVP_CIPHER_CTX_new()))
        {
            std::cerr << "EVP_CIPHER_CTX_new error" << std::endl;
        }

        // Initialize decryptor
        if (1 != EVP_DecryptInit_ex(decrypt_ctx, EVP_aes_128_cfb8(), NULL, key, iv))
        {
            std::cerr << "EVP_DecryptInit_ex error" << std::endl;
        }

        this->encryption_enabled = true;
    }

    void Socket::CleanupEncryption()
    {
        if (this->encryption_enabled)
        {
            this->encryption_enabled = false;
            EVP_CIPHER_CTX_free(encrypt_ctx);
            EVP_CIPHER_CTX_free(decrypt_ctx);
        }
    }

    // Encrypt using AES-128 CFB8
    int Socket::Encrypt(uint8_t* decrypted_bytes, int decrypted_len, uint8_t* encrypted_bytes)
    {
        int len;
        if (ASSERT_TRUE(EVP_EncryptUpdate(encrypt_ctx, encrypted_bytes, &len, decrypted_bytes, decrypted_len) == 1, "EVP_EncryptUpdate() error: " + get_openssl_error_string()))
        {
            return -1;
        }

        return len;
    }

    // Decrypt AES-128 CFB8 text
    int Socket::Decrypt(uint8_t* encrypted_bytes, int encrypted_len, uint8_t* decrypted_bytes)
    {
        int len;
        if(ASSERT_TRUE(EVP_DecryptUpdate(decrypt_ctx, decrypted_bytes, &len, encrypted_bytes, encrypted_len) == 1, "EVP_DecryptUpdate() error: " + get_openssl_error_string()))
        {
            return -1;
        }

        return len;
    }

    int Socket::Decompress(uint8_t* compressed_bytes, int compressed_length, uint8_t* decompressed_bytes, int decompressed_length)
    {
        // Configure Stream
        z_stream stream;
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;
        stream.avail_in = (uInt)compressed_length;
        stream.next_in = (Bytef*)compressed_bytes;
        stream.avail_out = (uInt)decompressed_length;
        stream.next_out = (Bytef*)decompressed_bytes;

        // Decompress
        if (ASSERT_TRUE(inflateInit(&stream) == Z_OK, "inflateInit() Error"))
        {
            return -1;
        }

        if (ASSERT_TRUE(inflate(&stream, Z_NO_FLUSH) != Z_STREAM_ERROR, "inflate() error"))
        {
            return -1;
        }

        inflateEnd(&stream);
        return Z_OK;
    }

    int Socket::Compress(uint8_t* compressed_bytes, int compressed_length, uint8_t* decompressed_bytes, int decompressed_length)
    {
        // Configure Stream
        z_stream stream;
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;
        stream.avail_in = (uInt)decompressed_length;
        stream.next_in = (Bytef*)decompressed_bytes;
        stream.avail_out = (uInt)compressed_length;
        stream.next_out = (Bytef*)compressed_bytes;

        // Compress
        if (ASSERT_TRUE(deflateInit(&stream, Z_DEFAULT_COMPRESSION) == Z_OK, "deflateInit() error"))
        {
            return -1;
        }

        if(ASSERT_TRUE(deflate(&stream, Z_NO_FLUSH) != Z_STREAM_ERROR, "deflate() error"))
        {
            return -1;
        }

        deflateEnd(&stream);
        return Z_OK;
    }

    void Socket::InitCompression(int max_decompressed_length)
    {
        this->max_uncompressed_length = max_decompressed_length;
        this->compression_enabled = true;
    }

    int Socket::ConnectSocket(addrinfo* info)
    {
        return connect(this->socket, info->ai_addr, (int)info->ai_addrlen);
    }

    int Socket::ReadPacket(uint8_t* packet, int length, int decompressed_length)
    {
        if (this->encryption_enabled)
        {
            uint8_t* encrypted_packet = new uint8_t[length]{ 0 };
            uint8_t* decrypted_packet = new uint8_t[length]{ 0 };
            int bytes_read = recv(this->socket, (char*)encrypted_packet, length, 0);

            // Decrypt Packet //
            int decrypted_packet_length = Decrypt((unsigned char*)encrypted_packet, length, decrypted_packet);
            delete[] encrypted_packet;
            if (ASSERT_TRUE(decrypted_packet_length >= 0, "Failed to decrypt packet"))
            {
                delete[] decrypted_packet;
                return -1;
            }

            // Decompress Packet (if compressed) //
            if (this->compression_enabled && (length > 1) && (decompressed_length != 0))
            {
                uint8_t* decompressed_packet = new uint8_t[decompressed_length]{ 0 };

                if (ASSERT_TRUE(Decompress(decrypted_packet, length, decompressed_packet, decompressed_length) >= 0, "Failed to decompress packet"))
                {
                    delete[] decrypted_packet;
                    delete[] decompressed_packet;
                    return -1;
                }

                std::copy(decompressed_packet, decompressed_packet + decompressed_length, packet);
                delete[] decrypted_packet;
                delete[] decompressed_packet;
                return decompressed_length;
            }
            else // Packet is not compressed
            {
                std::copy(decrypted_packet, decrypted_packet + decrypted_packet_length, packet);
                delete[] decrypted_packet;
                return decrypted_packet_length;
            }

        }
        else
        {
            // TODO: implement compression here
            return recv(this->socket, (char*)packet, length, 0);
        }
    }

    int Socket::SendPacket(uint8_t* packet, size_t length)
    {
        uint8_t* header = NULL;
        int header_length;
        size_t offset = 0;

        // Create Header //
        if (this->compression_enabled)
        {
            int data_length;
            int packet_length;

            if (length > this->max_uncompressed_length)
            {
                // TODO: handle compressed fields
                data_length = length;
            }
            else
            {
                data_length = 0;
                packet_length = PacketEncoder::GetVarIntNumBytes(data_length) + length;
            }

            header_length = PacketEncoder::GetVarIntNumBytes(packet_length) + PacketEncoder::GetVarIntNumBytes(data_length);
            header = new uint8_t[header_length]{ 0 };
            PacketEncoder::WriteVarInt(packet_length, header, offset);
            PacketEncoder::WriteVarInt(data_length, header, offset);
        }
        else
        {
            header_length = PacketEncoder::GetVarIntNumBytes(length);
            header = new uint8_t[header_length]{ 0 };
            PacketEncoder::WriteVarInt(length, header, offset);
        }

        // Put Header On Top //
        int full_length = header_length + length;
        uint8_t* full_packet = new uint8_t[full_length]{ 0 };
        std::copy(header, header + header_length, full_packet);
        std::copy(packet, packet + length, full_packet + header_length);

        // Send Packet
        if (this->encryption_enabled)
        {
            // Encrypt //
            uint8_t* encrypted_packet = new uint8_t[full_length]{ 0 };
            int encrypted_length = Encrypt((uint8_t*)full_packet, full_length, encrypted_packet);
            delete[] full_packet;
            if (ASSERT_TRUE(encrypted_length >= 0, "Failed to encrypt packet"))
            {
                delete[] encrypted_packet;
                return -1;
            }

            // TODO: handle compression

            // Send //
            int ret = send(this->socket, (char*)encrypted_packet, encrypted_length, 0);
            delete[] encrypted_packet;
            return ret;
        }
        else
        {
            int ret = send(this->socket, (char*)full_packet, full_length, 0);
            delete[] full_packet;
            return  ret;
        }
    }
}

