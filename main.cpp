// MCBot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <sstream>
#include <random>

#include "lib/httplib.h"
#include "MCBot.h"
#include "JsonObject.h"
#include "DaftHash.h"

using namespace mcbot;

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Must enter username and password" << std::endl;
        return -1;
    }

    char* email = argv[1];
    char* password = argv[2];
    mcbot::MCBot bot(email, password);
    bot.set_debug(false);

    // Log in to Mojang auth servers
    // - To resolve our email to a username and a UUID
    // - To obtain an access token
    std::cout << "Logging in to Mojang authservers..." << std::endl;
    if (bot.login_mojang() < 0)
    {
        std::cerr << "Failed to log in to Mojang authservers!" << std::endl;
        return -1;
    }

    std::cout << "Verifying access token..." << std::endl;
    if (bot.verify_access_token() < 0)
    {
        std::cerr << "Invalid access token!" << std::endl;
        return -1;
    }

    // Connect to server
    // - So we can send it packets
    char* hostname = (char*) "moxmc.net";   
    char* port = (char*) "25565";
    if (bot.connect_server(hostname, port) < 0)
    {
        std::cout << "Failed to join server" << std::endl;
        return -1;
    }

    bot.send_handshake(hostname, atoi(port));
    bot.send_login_start();

    while (bot.is_connected())
    {
        Sleep(1);
        bot.recv_packet();
    }
}
