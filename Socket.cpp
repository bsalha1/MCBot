#include "Socket.h"
#include "zlib.h"

void write_var_int(int value, uint8_t* packet, size_t& offset)
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

int get_var_int_size(int value)
{
    int size = 0;
    do
    {
        char temp = (char)(value & 0b01111111);
        value >>= 7;
        if (value != 0)
        {
            temp |= 0b10000000;
        }
        size++;
    } while (value != 0);
    return size;
}

mcbot::Socket::Socket(SOCKET socket)
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

mcbot::Socket::Socket()
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

mcbot::Socket::~Socket()
{
    if (this->encryption_enabled)
    {
        this->cleanup_encryption();
    }
}

void mcbot::Socket::initialize_encryption(uint8_t* key, uint8_t* iv)
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

void mcbot::Socket::cleanup_encryption()
{
    this->encryption_enabled = false;
    EVP_CIPHER_CTX_free(encrypt_ctx);
    EVP_CIPHER_CTX_free(decrypt_ctx);
}

// Encrypt using AES-128 CFB8
int mcbot::Socket::encrypt(uint8_t* decrypted_text, int decrypted_len, uint8_t* encrypted_text)
{
    int len;
    int encrypted_len;
    if (EVP_EncryptUpdate(encrypt_ctx, encrypted_text, &len, decrypted_text, decrypted_len) != 1)
    {
        std::cerr << "EVP_EncryptUpdate error" << std::endl;
    }
    encrypted_len = len;

    return encrypted_len;
}

// Decrypt AES-128 CFB8 text
int mcbot::Socket::decrypt(uint8_t* encrypted_text, int encrypted_len, uint8_t* decrypted_text)
{
    int len;
    int decrypted_len;
    if (EVP_DecryptUpdate(decrypt_ctx, decrypted_text, &len, encrypted_text, encrypted_len) != 1)
    {
        std::cerr << "EVP_DecryptUpdate error" << std::endl;
    }
    decrypted_len = len;

    return decrypted_len;
}

int mcbot::Socket::decompress(uint8_t* compressed, int compressed_length, uint8_t* decompressed, int decompressed_length)
{
    // Configure Stream
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = (uInt)compressed_length;
    stream.next_in = (Bytef*)compressed;
    stream.avail_out = (uInt)decompressed_length;
    stream.next_out = (Bytef*)decompressed;

    // Decompress
    int ret = inflateInit(&stream);
    if (ret != Z_OK)
    {
        return ret;
    }

    ret = inflate(&stream, Z_NO_FLUSH);
    if (ret == Z_STREAM_ERROR)
    {
        return ret;
    }
    inflateEnd(&stream);
    return Z_OK;
}

int mcbot::Socket::compress(uint8_t* compressed, int compressed_length, uint8_t* decompressed, int decompressed_length)
{
    // Configure Stream
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = (uInt)decompressed_length;
    stream.next_in = (Bytef*)decompressed;
    stream.avail_out = (uInt)compressed_length;
    stream.next_out = (Bytef*)compressed;

    // Compress
    int ret = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK)
    {
        return ret;
    }
    
    ret = deflate(&stream, Z_NO_FLUSH);
    if (ret == Z_STREAM_ERROR)
    {
        return ret;
    }
    deflateEnd(&stream);
    return Z_OK;
}

void mcbot::Socket::initialize_compression(int max_decompressed_length)
{
    this->max_uncompressed_length = max_decompressed_length;
    this->compression_enabled = true;
}

int mcbot::Socket::recv_packet(uint8_t* packet, int length, int decompressed_length)
{
    if (this->encryption_enabled)
    {
        uint8_t* encrypted_packet = (uint8_t*) calloc(length, sizeof(uint8_t));
        uint8_t* decrypted_packet = (uint8_t*) calloc(length, sizeof(uint8_t));
        int bytes_read = recv(this->socket, (char*)encrypted_packet, length, 0);

        int decrypted_packet_length = decrypt((unsigned char*)encrypted_packet, length, decrypted_packet);
        free(encrypted_packet);

        if (this->compression_enabled && length > 1)
        {
            if (decompressed_length == 0)
            {
                memcpy(packet, decrypted_packet, decrypted_packet_length);
                free(decrypted_packet);
            }
            else
            {
                uint8_t* decompressed_packet = (uint8_t*)calloc(decompressed_length, sizeof(uint8_t));

                decompress(decrypted_packet, length, decompressed_packet, decompressed_length);
                free(decrypted_packet);

                memcpy(packet, decompressed_packet, decompressed_length);
                free(decompressed_packet);

                return decompressed_length;
            }
        }
        else
        {
            memcpy(packet, decrypted_packet, decrypted_packet_length);
            free(decrypted_packet);
        }
        return decrypted_packet_length;

    }
    else
    {
        if (this->compression_enabled)
        {
            // TODO
        }

        return recv(this->socket, (char*) packet, length, 0);
    }
}

int mcbot::Socket::send_pack(uint8_t* packet, int length)
{
    if (this->encryption_enabled)
    {
        if (this->compression_enabled)
        {
            int data_length;
            int packet_length;

            if (length > this->max_uncompressed_length)
            {
                uint8_t* compressed_packet = (uint8_t *) calloc(length, sizeof(uint8_t));
                compress(compressed_packet, length, packet, length);

                uint8_t* encrypted_packet = (uint8_t*)calloc(length, sizeof(uint8_t));
                int encrypted_length = encrypt(compressed_packet, length, encrypted_packet);

                data_length = length;
            }
            else
            {
                // Create Header
                data_length = 0;
                packet_length = get_var_int_size(data_length) + length;

                int header_length = get_var_int_size(packet_length) + get_var_int_size(data_length);
                uint8_t* header = new uint8_t[header_length]{ 0 };
                size_t offset = 0;

                write_var_int(packet_length, header, offset);
                write_var_int(data_length, header, offset);

                // Encrypt Header
                uint8_t* encrypted_header = new uint8_t[header_length]{ 0 };
                int encrypted_header_length = encrypt(header, header_length, encrypted_header);

                // Encrypt Data
                uint8_t* encrypted_packet = new uint8_t[length]{ 0 };
                int encrypted_length = encrypt(packet, length, encrypted_packet);

                // Put Header on Top of Data
                uint8_t* full_packet = new uint8_t[encrypted_header_length + encrypted_length]{ 0 };
                for (int i = 0; i < encrypted_header_length + encrypted_length; i++)
                {
                    if (i < encrypted_header_length)
                    {
                        full_packet[i] = encrypted_header[i];
                    }
                    else
                    {
                        full_packet[i] = encrypted_packet[i - encrypted_header_length];
                    }
                }


                // Send Header //
                int ret = send(this->socket, (char*)full_packet, encrypted_header_length + encrypted_length, 0);
                if (ret < 0)
                {
                    std::cerr << "Failed to send header packet" << std::endl;
                    delete[] header;
                    delete[] encrypted_header;
                    delete[] encrypted_packet;
                    delete[] full_packet;
                    return ret;
                }

                delete[] header;
                delete[] encrypted_header;
                delete[] encrypted_packet;
                delete[] full_packet;
                return ret;
            }
        }

        char encrypted_packet[1028] = { 0 };
        int encrypted_len = encrypt((uint8_t*)packet, length, (uint8_t*)encrypted_packet);
        return send(this->socket, encrypted_packet, encrypted_len, 0);
    }
    else
    {
        if (this->compression_enabled)
        {
            // TODO
        }

        // Send Header
        int header_length = get_var_int_size(length);
        uint8_t* header = (uint8_t*)calloc(header_length, sizeof(uint8_t));
        size_t offset = 0;

        write_var_int(length, header, offset);
        send(this->socket, (char*)header, header_length, 0);
        free(header);

        return send(this->socket, (char*)packet, length, 0);
    }
}

int mcbot::Socket::connect_socket(addrinfo* info)
{
    return connect(this->socket, info->ai_addr, (int)info->ai_addrlen);
}

