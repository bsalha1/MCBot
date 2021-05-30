#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <iostream>

#include "lib/httplib.h"
#include <openssl/rsa.h>
#include <openssl/aes.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>

namespace mcbot
{
	class Socket
	{
	private:
		SOCKET socket;

		uint8_t* key;
		uint8_t* iv;
		EVP_CIPHER_CTX* encrypt_ctx;
		EVP_CIPHER_CTX* decrypt_ctx;
		bool encryption_enabled;

		int max_uncompressed_length;
		bool compression_enabled;

	public:
		Socket(SOCKET socket);
		Socket();
		~Socket();

		// Encryption
		void InitEncryption(uint8_t* key, uint8_t* iv);
		void CleanupEncryption();
		int Encrypt(uint8_t* decrypted_text, int decrypted_len, uint8_t* encrypted_text);
		int Decrypt(uint8_t* encrypted_text, int encrypted_len, uint8_t* decrypted_text);

		// Compression
		void InitCompression(int max_uncompressed_length);
		int Decompress(uint8_t* compressed, int compressed_length, uint8_t* decompressed, int decompressed_length);
		int Compress(uint8_t* compressed, int compressed_length, uint8_t* decompressed, int decompressed_length);

		int RecvPacket(uint8_t* packet, int length, int decompressed_length = 0);
		int SendPacket(uint8_t* packet, size_t length);
		int ConnectSocket(addrinfo* info);
	};
}

