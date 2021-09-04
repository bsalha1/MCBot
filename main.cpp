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

using namespace McBot;
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

    MCBot bot;
    bot.GetLogger().SetDebug(false);


    // Log in to Mojang auth servers
    // - To resolve our email to a username and a UUID
    // - To obtain an access token
    if (bot.LoginToMojang(email, password) < 0)
    {
        bot.GetLogger().LogError("Failed to log in to Mojang");
        return -1;
    }
    

    // Connect to TCP Server
    if (bot.ConnectToServer(hostname, port) < 0)
    {
        bot.GetLogger().LogError("Failed to connect to server");
        return -1;
    }


    // Log in to Minecraft Server
    bot.LoginToServer(hostname, port);


    // Receive packets until disconnected
    std::thread recv_thread = bot.StartPacketReceiverThread();


    // Send player's position every tick
    std::thread position_thread = bot.StartPositionThread();

    // Do specific tasks
    std::thread tick_thread([&bot]() {

        bot.AwaitState(State::PLAY);

        bot.GetPacketSender().SendSettings();
        bot.GetPacketSender().SendCustomPayload("vanilla");

        Sleep(1000);

        // Move to better center of a block to make math easier
        McBot::Vector<double> clean_position = bot.GetPlayer().GetLocation();
        clean_position.Floor();
        clean_position = clean_position + McBot::Vector<double>(0.5, 0, 0.5);
        bot.MoveTo(clean_position, 1.0);

        bot.GetPacketSender().SendArmAnimation();

        int target_id;
        for (Entity entity : bot.GetEntityRegistry().GetValues())
        {
            if (entity.GetEntityType() == EntityType::MUSHROOM_COW)
            {
                std::cout << "Found mushroom cow: " << entity.GetID() << std::endl;
                target_id = entity.GetID();
                break;
            }
        }

        Entity& target = bot.GetEntityRegistry().GetValue(target_id);

        while (true)
        {
            auto target_location = target.GetLocation();
            bot.MoveTo(target_location.GetX(), target_location.GetZ(), 5);
        }

        //mcbot::Vector<double> loc = bot.GetPlayer().GetLocation();
        //mcbot::Vector<int> loc1 = mcbot::Vector<int>(loc.GetX(), loc.GetY(), loc.GetZ());

        //auto items = bot.GetPlayer().GetInventory();
        //mcbot::Slot best_axe;
        //for (auto item : items)
        //{
        //    // Diamond Axe
        //    if (item.GetID() == 279)
        //    {
        //        best_axe = item;
        //    }

        //    // Iron Axe
        //    if (item.GetID() == 258 && best_axe.GetID() != 279)
        //    {
        //        best_axe = item;
        //    }

        //    // Stone Axe
        //    if (item.GetID() == 275 && best_axe.GetID() != 279 && best_axe.GetID() != 258)
        //    {
        //        best_axe = item;
        //    }

        //    // Wood Axe
        //    if (item.GetID() == 271 && best_axe.GetID() != 279 && best_axe.GetID() != 258 && best_axe.GetID() != 275)
        //    {
        //        best_axe = item;
        //    }
        //}

        //// No axe found
        //if (best_axe.GetID() == -1)
        //{
        //    std::cout << "No axe found" << std::endl;
        //}

        //
        //auto coords_list = std::list<mcbot::Vector<int>>();
        //auto chunks = bot.GetChunks(loc1.GetX() >> 4, loc1.GetZ() >> 4, 1);
        //for (auto ch : chunks)
        //{
        //    auto coords = ch.GetBlockCoordinates(17);
        //    coords_list.insert(coords_list.end(), coords.begin(), coords.end());
        //}

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
