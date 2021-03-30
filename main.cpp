// MCBot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <iostream>
#include <string>
#include <sstream>
#include <random>

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "lib/httplib.h"
#include "JsonObject.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace mcbot;

void print_array(char* arr, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        printf("%d ", (unsigned char) arr[i]);
    }
    printf("\n");
}

void print_winsock_error()
{

    wchar_t* s = NULL;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&s, 0, NULL);
    printf("%S\n", s);
}

int read_var_int(char* bytes, size_t* offset)
{
    int num_read = 0;
    int result = 0;
    char read;
    int i = 0;
    do {
        read = bytes[(*offset) + num_read];
        int value = (read & 0b01111111);
        result |= (value << (7 * num_read));

        num_read++;
        if (num_read > 5)
        {
            *offset += num_read;
            fprintf(stderr, "VarInt out of bounds");
            return -1;
        }
    } while ((read & 0b10000000) != 0);

    *offset += num_read;
    return result;
}

int read_string_n(char* string, size_t string_length, char* packet, size_t packet_size, size_t* offset)
{
    int n = read_var_int(packet, offset);
    for (size_t i = 0; i < n && i < string_length; i++)
    {
        string[i] = packet[(*offset)++];
    }
    return n;
}

void read_byte_array(char* bytes, int bytes_length, char* packet, size_t packet_size, size_t* offset)
{
    for (int i = 0; i < bytes_length; i++)
    {
        bytes[i] = packet[(*offset)++];
    }
}

void write_var_int(int value, char * packet, size_t packet_size, size_t* offset)
{
    do
    {
        char temp = (char)(value & 0b01111111);
        value >>= 7;
        if (value != 0)
        {
            temp |= 0b10000000;
        }
        packet[(*offset)++] = temp;
    } while (value != 0 && (*offset) <= packet_size);
}

size_t get_var_int_size(int value)
{
    size_t size = 0;
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

void write_string_n(char* string, char* packet, size_t packet_size, size_t* offset)
{
    size_t string_length = strlen(string);
    write_var_int(string_length, packet, packet_size, offset);
    
    for (int i = 0; i < string_length; i++)
    {
        packet[(*offset)++] = string[i];
    }
}

void write_ushort(unsigned short num, char* packet, size_t packet_size, size_t* offset)
{
    packet[(*offset)++] = num >> 8;
    packet[(*offset)++] = num & 0xFF;
}

void write_packet_length(char* packet, size_t packet_size, size_t* offset)
{
    int length = *offset;
    int packet_length_size = get_var_int_size(length);

    for (int i = length - 1; i >= 0; i--)
    {
        packet[i + packet_length_size] = packet[i];
    }

    *offset += packet_length_size;
    size_t offset2 = 0;
    write_var_int(length, packet, sizeof(packet), &offset2);
}

void send_handshake(SOCKET sock, char* hostname, unsigned short port)
{
    char packet[1028];
    size_t offset = 0;

    write_var_int(0x00, packet, sizeof(packet), &offset); // packet id
    write_var_int(47, packet, sizeof(packet), &offset);   // protocol version
    write_string_n(hostname, packet, sizeof(packet), &offset); // hostname
    write_ushort(port, packet, sizeof(packet), &offset); // port
    write_var_int(2, packet, sizeof(packet), &offset);    // next state
    write_packet_length(packet, sizeof(packet), &offset);

    if (send(sock, packet, offset, 0) < 0)
    {
        std::cout << "Failed to send packet" << std::endl;
        print_winsock_error();
    }
}

void send_login_start(SOCKET sock)
{
    char packet[1028];
    size_t offset = 0;

    write_var_int(0x00, packet, sizeof(packet), &offset); // packet id
    write_string_n((char*)"ReachCarter", packet, sizeof(packet), &offset); // username
    write_packet_length(packet, sizeof(packet), &offset);

    if (send(sock, packet, offset, 0) < 0)
    {
        std::cout << "Failed to send packet" << std::endl;
        print_winsock_error();
    }
}

void recv_encryption_request(SOCKET sock, char* packet, size_t size_read, size_t* offset)
{
    std::cout << "\tHandling Encryption Request..." << std::endl;

    char server_id[64] = { 0 };
    read_string_n(server_id, sizeof(server_id), packet, size_read, offset);
    printf("Server ID: %s\n", server_id);

    int public_key_length = read_var_int(packet, offset);
    char* public_key = (char*)malloc(public_key_length);
    read_byte_array(public_key, public_key_length, packet, size_read, offset);
    printf("Public key: ");
    print_array(public_key, public_key_length);
    

    int verify_token_length = read_var_int(packet, offset);
    char* verify_token = (char*)malloc(verify_token_length);
    read_byte_array(verify_token, verify_token_length, packet, size_read, offset);
    printf("Verify token: ");
    print_array(verify_token, verify_token_length);

    printf("%ld\n", *offset);
}

void recv_packet(SOCKET sock)
{
    char packet[1028] = { 0 };
    size_t bytes_read = recv(sock, packet, 1028, 0);
    if (bytes_read < 0)
    {
        std::cout << "Failed to receive packet" << std::endl;
        return;
    }
    std::cout << "Received Packet: " << bytes_read << "bytes" <<  std::endl;

    size_t offset = 0;
    int length = read_var_int(packet, &offset);
    int packet_id = read_var_int(packet, &offset);
    std::cout << "\tLength: " << length << std::endl << "\tPacket ID: " << packet_id << std::endl;
    print_array(packet, bytes_read);
    switch (packet_id)
    {
        case 0x01:
            recv_encryption_request(sock, packet, bytes_read, &offset);
            break;
        default:
            std::cout << "Unhandled packet ID: " << packet_id << std::endl;
    }
}

int join_server(char* hostname, char* port)
{
    // Resolve Host //
    struct addrinfo hints;
    struct addrinfo* result;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if (getaddrinfo(hostname, port, &hints, &result) < 0)
    {
        std::cout << "Failed to resolve " << hostname << std::endl;
        print_winsock_error();
        WSACleanup();
        return -1;
    }
    char* address_string = inet_ntoa(((struct sockaddr_in*) result->ai_addr)->sin_addr);
    std::cout << "Resolved " << hostname << " to " << address_string << std::endl;

    // Connect //
    SOCKET sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    std::cout << "Connecting to " << address_string << ":" << port << std::endl;
    if (connect(sock, result->ai_addr, (int)result->ai_addrlen) < 0)
    {
        print_winsock_error();
        WSACleanup();
        return -1;
    }
    std::cout << "Connected to " << address_string << ":" << port << std::endl;

    return sock;
}



int account_login(char* username, char* password)
{
    char url[] = "https://authserver.mojang.com";
 
    // Payload //
    char content_format[] = "{\"agent\": {"
        "\"name\": \"Minecraft\","
        "\"version\" : 1 },"
        "\"username\" : \"%s\","
        "\"password\" : \"%s\","
        "\"requestUser\" : true}\r\n";
    char content[256] = { 0 };
    sprintf_s(content, content_format, username, password);

    // Send Payload //
    httplib::Client cli(url);
    httplib::Result res = cli.Post("/authenticate", content, "application/json");
    std::string response = res->body;

    std::size_t user_index = response.rfind("\"user\":") + strlen("\"user\":");

    auto json = mcbot::JsonObject::serialize(response);
    std::cout << json << std::endl;
   
    return 0;
}

int main()
{
    // Start WinSock DLL //
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout << "Failed to start up WinSock DLL" << std::endl;
        print_winsock_error();
        return -1;
    }
    std::cout << "WinSock DLL Started" << std::endl;

    char* username = (char*) "ReachCarter";
    char* password = (char*) "Kll4403130!";
    account_login((char*) "leopardgeckochannel@gmail.com", password);

    char* hostname = (char*) "cosmicpvp.com";   
    char* port = (char*) "25565";
    int sock = join_server(hostname, port);
    if (sock < 0)
    {
        std::cout << "Failed to join server" << std::endl;
        return -1;
    }

    send_handshake(sock, hostname, atoi(port));
    send_login_start(sock);

    Sleep(100);
    recv_packet(sock);

    

}
