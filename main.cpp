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
#include "ScriptRuntime.h"

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
        bot.send_position(bot.get_player().get_location(), true);
        bot.send_position(bot.get_player().get_location(), true);

        Entity target;
        for (Entity entity : bot.get_entities())
        {
            if (entity.get_entity_type() == EntityType::PIG)
            {
                std::cout << "Found pig" << std::endl;
                target = entity;
                break;
            }
        }

        bot.move_to_ground();

        bot.move_to(mcbot::Vector<double>(454, 70, 150), 0.50);


        //std::ofstream script_file("script.txt");
        //script_file << "move_rel({1,0,0})" << std::endl;
        //script_file << "move_rel({-1,0,0})" << std::endl;
        //script_file << "move_rel({0,0,1})" << std::endl;
        //script_file << "move_rel({0,0,-1})" << std::endl;
        //script_file.close();

        //std::list<std::string> commands;
        //std::fstream newfile;
        //newfile.open("script.txt", std::ios::in);
        //if (newfile.is_open())
        //{
        //    std::string line;
        //    while (getline(newfile, line))
        //    {
        //        std::string::iterator end_pos = std::remove(line.begin(), line.end(), ' ');
        //        line.erase(end_pos, line.end());
        //        commands.push_back(line);
        //    }
        //    newfile.close();
        //}

        //mcbot::ScriptRuntime script_runtime = mcbot::ScriptRuntime(bot);
        //while (bot.is_connected())
        //{
        //    for (std::string command : commands)
        //    {
        //        std::string method = command.substr(0, command.find_first_of('('));
        //        std::string args_string = command.substr(command.find_first_of('(') + 1, command.find_last_of(')') - (command.find_first_of('(') + 1));

        //        std::list<std::string> args;
        //        int i = 0;
        //        do 
        //        {
        //            if (args_string[i] == '{')
        //            {
        //                std::string vector = "";
        //                while (args_string[i] != '}')
        //                {
        //                    vector += args_string[i++];
        //                }
        //                vector += "}";
        //                args.push_back(vector);
        //            }
        //            else
        //            {
        //                std::string arg = "";
        //                while (args_string[i] != ',' && i < args_string.length())
        //                {
        //                    arg += args_string[i++];
        //                }

        //                if (arg.length() != 0)
        //                {
        //                    args.push_back(arg);
        //                }
        //            }
        //            i++;
        //        } while (i < args_string.length());

        //        script_runtime.call_method(method, args);
        //        Sleep(1000 / 20);
        //    }
        //}
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
