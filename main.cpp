// MCBot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <sstream>
#include <random>
#include <thread>
#include <future>

#include "lib/httplib.h"
#include "MCBot.h"
#include "DaftHash.h"

using namespace mcbot;
using namespace std::literals;

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        std::cerr << "Must enter username, password and IP address of server" << std::endl;
        return -1;
    }

    char* email = argv[1];
    char* password = argv[2];
    char* hostname = argv[3];
    char* port = (argc >= 5) ? argv[4] : (char*)"25565";

    MCBot bot(email, password);
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
    if (bot.connect_server(hostname, port) < 0)
    {
        std::cout << "Failed to join server" << std::endl;
        return -1;
    }

    bot.send_handshake(hostname, atoi(port));
    bot.send_login_start();

    std::thread recv_thread([&bot]() {
        while (bot.is_connected())
        {
            Sleep(1);
            bot.recv_packet();
        }
    });

    std::thread tick_thread([&bot]() {

        Sleep(2000);

        std::cout << "Sending settings" << std::endl;
        bot.send_settings();
        bot.send_held_item_slot(0);
        bot.send_custom_payload("vanilla");

        std::cout << "Sending positions" << std::endl;
        bot.send_position(bot.get_player().get_position(), true);
        bot.send_position(bot.get_player().get_position(), true);

        double angle = 0;
        while (bot.is_connected())
        {
            double x = 0.2*cos(angle);
            double y = 0.2*sin(angle);
            bot.send_position_look(bot.get_player().get_position().sum(x, 0, y), angle / 3.14 * 180, 0, true);
            Sleep(50);

            angle += 3.14 / 20.0;

            if (angle >= 2 * 3.14)
            {
                angle = 0;
            }
        }
    });

    do
    {
        std::string input;
        std::getline(std::cin, input);
        if (!bot.is_connected())
        {
            break;
        }
        bot.send_chat_message(input);

    } while (bot.is_connected());


    return 0;
}
