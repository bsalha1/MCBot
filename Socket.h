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

#include "Assert.h"

namespace McBot
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

		// Encryption //
		void InitEncryption(uint8_t* key, uint8_t* iv);
		void CleanupEncryption();
		int Encrypt(uint8_t* decrypted_bytes, int decrypted_len, uint8_t* encrypted_bytes);
		int Decrypt(uint8_t* encrypted_bytes, int encrypted_len, uint8_t* decrypted_bytes);

		// Compression //
		void InitCompression(int max_uncompressed_length);
		int Decompress(uint8_t* compressed_bytes, int compressed_length, uint8_t* decompressed_bytes, int decompressed_length);
		int Compress(uint8_t* compressed_bytes, int compressed_length, uint8_t* decompressed_bytes, int decompressed_length);

		/*
			Connects "this->socket" to host given the "addrinfo" struct.
			@param info: the address info
			@return the error code of connect()
		*/
		int ConnectSocket(addrinfo* info);


		/*
			Reads, decrypts and then decompresses "length" bytes from the packet stream (if encryption and compression are enabled).
			@param packet: the output packet
			@param length: number of bytes to read from packet stream
			@param decompressed_length: number of bytes of packet when it is decompressed (if 0 then it is not compressed so it does not need to be decompressed)
			@return number of bytes the output packet has
		*/
		int ReadPacket(uint8_t* packet, int length, int decompressed_length = 0);


		/*
			Compresses, encrypts and then sends "length" bytes of "packet" (if encryption and compression are enabled and compresses only if size of packet is greater than "this->max_uncompressed_length").
			@param packet: packet to send
			@param length: size of packet to send
			@return size of packet sent
		*/
		int SendPacket(uint8_t* packet, size_t length);
	};
}

