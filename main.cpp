// MCBot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <sstream>
#include <random>

#include <stdlib.h>
#include <stdio.h>

#include "lib/httplib.h"
#include "MCBot.h"
#include "JsonObject.h"

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

    // Log in to Mojang authservers
    // - To resolve our email to a username
    // - To obtain an access token
    std::cout << "Logging into Mojang authservers with " << email << "..." << std::endl;
    bot.mojang_login();

    // Connect to server
    // - So we can send it packets
    char* hostname = (char*) "cosmicpvp.com";   
    char* port = (char*) "25565";
    int sock = bot.connect_server(hostname, port);
    if (sock < 0)
    {
        std::cout << "Failed to join server" << std::endl;
        return -1;
    }

    bot.send_handshake(hostname, atoi(port));
    bot.send_login_start();

    Sleep(100);
    bot.recv_packet();

    

}
