#include "Socket.h"

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

    //if (1 != EVP_EncryptFinal_ex(encrypt_ctx, encrypted_text + len, &len))
    //{
    //    std::cerr << "EVP_EncryptFinal_ex error" << std::endl;
    //}
    //encrypted_len += len;

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

    //if (1 != EVP_DecryptFinal_ex(decrypt_ctx, decrypted_text + len, &len))
    //{
    //    std::cerr << "EVP_DecryptFinal_ex error" << std::endl;
    //}
    //decrypted_len += len;

    return decrypted_len;
}

void mcbot::Socket::initialize_compression(int max_uncompressed_length)
{
    this->max_uncompressed_length = max_uncompressed_length;
}

int mcbot::Socket::recv_packet(uint8_t* packet, int length)
{
    if (this->encryption_enabled)
    {
        std::cout << "Receiving encrypted..." << std::endl;
        uint8_t* encrypted_packet = (uint8_t*) calloc(length, sizeof(uint8_t));
        int bytes_read = recv(this->socket, (char*)encrypted_packet, length, 0);

        int out_len = decrypt((unsigned char*)encrypted_packet, length, packet);

        if (this->compression_enabled)
        {

        }

        free(encrypted_packet);
        return out_len;
    }
    else
    {
        std::cout << "Receiving..." << std::endl;
        return recv(this->socket, (char*) packet, length, 0);
    }
}

int mcbot::Socket::send_pack(uint8_t* packet, int length)
{
    if (this->encryption_enabled)
    {
        std::cout << "Sending encrypted..." << std::endl;
        char encrypted_packet[1028] = { 0 };
        int encrypted_len = encrypt((uint8_t*)packet, length, (uint8_t*)encrypted_packet);
        send(this->socket, encrypted_packet, encrypted_len, 0);
    }
    else
    {
        std::cout << "Sending..." << std::endl;
        return send(this->socket, (char*)packet, length, 0);
    }
}

int mcbot::Socket::connect_socket(addrinfo* info)
{
    return connect(this->socket, info->ai_addr, (int)info->ai_addrlen);
}

