#include "Socket.h"
#include "zlib.h"

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
    if (1 != EVP_EncryptUpdate(encrypt_ctx, encrypted_text, &len, decrypted_text, decrypted_len))
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
    if (1 != EVP_DecryptUpdate(decrypt_ctx, decrypted_text, &len, encrypted_text, encrypted_len))
    {
        std::cerr << "EVP_DecryptUpdate error" << std::endl;
    }
    decrypted_len = len;

    return decrypted_len;
}

void mcbot::Socket::decompress(uint8_t* compressed, int compressed_length, uint8_t* decompressed, int decompressed_length)
{
    // Configure Stream
    z_stream infstream;
    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.opaque = Z_NULL;
    infstream.avail_in = (uInt)compressed_length;
    infstream.next_in = (Bytef*)compressed;
    infstream.avail_out = (uInt)decompressed_length;
    infstream.next_out = (Bytef*)decompressed;

    // Decompress
    inflateInit(&infstream);
    inflate(&infstream, Z_NO_FLUSH);
    inflateEnd(&infstream);
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
                std::cout << "DECOMPRESSING" << std::endl;
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
            // TODO
        }

        char encrypted_packet[1028] = { 0 };
        int encrypted_len = encrypt((uint8_t*)packet, length, (uint8_t*)encrypted_packet);
        send(this->socket, encrypted_packet, encrypted_len, 0);
    }
    else
    {
        if (this->compression_enabled)
        {
            // TODO
        }

        return send(this->socket, (char*)packet, length, 0);
    }
}

int mcbot::Socket::connect_socket(addrinfo* info)
{
    return connect(this->socket, info->ai_addr, (int)info->ai_addrlen);
}

