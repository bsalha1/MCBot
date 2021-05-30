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
#include "PacketReceiver.h"
#include "PacketSender.h"
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

    MCBot bot = MCBot();
    bot.GetLogger().SetDebug(true);

    // Log in to Mojang auth servers
    // - To resolve our email to a username and a UUID
    // - To obtain an access token
    std::cout << "Logging in to Mojang authservers..." << std::endl;
    if (bot.GetPacketSender().LoginMojang(email, password) < 0)
    {
        std::cerr << "Failed to log in to Mojang authservers!" << std::endl;
        return -1;
    }

    std::cout << "Verifying access token..." << std::endl;
    if (bot.GetPacketSender().VerifyAccessToken() < 0)
    {
        std::cerr << "Invalid access token!" << std::endl;
        return -1;
    }

    // Connect to server
    // - So we can send it packets
    if (bot.ConnectToServer(hostname, port) < 0)
    {
        std::cout << "Failed to join server" << std::endl;
        return -1;
    }

    bot.GetPacketSender().SendHandshake(hostname, atoi(port));
    bot.GetPacketSender().SendLoginStart();

    // Receive packets
    std::thread recv_thread([&bot]() {
        while (bot.IsConnected())
        {
            Sleep(1);
            bot.GetPacketReceiver().RecvPacket();
        }
    });


    // Do specific tasks
    std::thread tick_thread([&bot]() {

        Sleep(4000);
        bot.GetPacketSender().SendSettings();
        bot.GetPacketSender().SendCustomPayload("vanilla");

        // Echo back player location (enables further sending of locations)
        bot.GetPacketSender().SendPosition(bot.GetPlayer().GetLocation(), true);

        Sleep(1000 / TPS);
        bot.GetPacketSender().SendPosition(bot.GetPlayer().GetLocation(), true);

        // Move to better center of a block to make math easier
        mcbot::Vector<double> clean_position = bot.GetPlayer().GetLocation();
        clean_position.Floor();
        clean_position = clean_position + mcbot::Vector<double>(0.5, 0, 0.5);

        Sleep(1000 / TPS);
        bot.GetPacketSender().SendPosition(clean_position, true);
        Sleep(1000 / TPS);
        bot.GetPacketSender().SendPosition(clean_position, true);

        bot.GetPacketSender().SendArmAnimation();

        int target_id = 0;
        for (Entity entity : bot.GetEntityRegistry().GetValues())
        {
            if (entity.GetEntityType() == EntityType::MUSHROOM_COW)
            {
                std::cout << "Found mushroom cow: " << entity.GetID() << std::endl;
                target_id = entity.GetID();
                break;
            }
        }

        //mcbot::Vector<double> target_location = target.get_location();

        //Entity& entity = bot.get_entity(target_id);
        //while (true)
        //{
        //    mcbot::Vector<double> target_location = entity.get_location();
        //    std::cout << target_location.to_string() << std::endl;
        //    bot.move_to(target_location.get_x(), target_location.get_z(), 4);
        //    Sleep(1000);
        //}
        //bot.attack_entity(target);

        mcbot::Vector<double> loc = bot.GetPlayer().GetLocation();
        mcbot::Vector<int> loc1 = mcbot::Vector<int>(loc.GetX(), loc.GetY(), loc.GetZ());

        auto items = bot.GetPlayer().GetInventory();
        mcbot::Slot best_axe;
        for (auto item : items)
        {
            // Diamond Axe
            if (item.GetID() == 279)
            {
                best_axe = item;
            }

            // Iron Axe
            if (item.GetID() == 258 && best_axe.GetID() != 279)
            {
                best_axe = item;
            }

            // Stone Axe
            if (item.GetID() == 275 && best_axe.GetID() != 279 && best_axe.GetID() != 258)
            {
                best_axe = item;
            }

            // Wood Axe
            if (item.GetID() == 271 && best_axe.GetID() != 279 && best_axe.GetID() != 258 && best_axe.GetID() != 275)
            {
                best_axe = item;
            }
        }

        // No axe found
        if (best_axe.GetID() == -1)
        {
            std::cout << "No axe found" << std::endl;
        }

        
        auto coords_list = std::list<mcbot::Vector<int>>();
        auto chunks = bot.GetChunks(loc1.GetX() >> 4, loc1.GetZ() >> 4, 1);
        for (auto ch : chunks)
        {
            auto coords = ch.GetBlockCoordinates(17);
            coords_list.insert(coords_list.end(), coords.begin(), coords.end());
        }

        //mcbot::Vector<int> closest_tree


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
        if (!bot.IsConnected())
        {
            break;
        }
        bot.GetPacketSender().SendChatMessage(input);

    } while (bot.IsConnected());


    return 0;
}
