#include "ScriptRuntime.h"

mcbot::Vector<double> get_vector(std::string vector_string)
{
    if (vector_string[0] != '{' || vector_string[vector_string.length() - 1] != '}')
    {
        std::cerr << "Vector missing \'{\' and/or \'}\'" << std::endl;
        return mcbot::Vector<double>();
    }

    std::string* elements = new std::string[3];

    int i = 1;
    int j = 0;
    while (i < vector_string.length() && vector_string[i] != '}')
    {
        std::string element = "";
        while (vector_string[i] != ',' && vector_string[i] != '}')
        {
            element += vector_string[i++];
        }

        elements[j++] = element;
        i++;
    }

    return mcbot::Vector<double>(std::stod(elements[0]), std::stod(elements[1]), std::stod(elements[2]));

}
mcbot::ScriptRuntime::ScriptRuntime(mcbot::MCBot& bot) : bot(bot)
{
    auto move_rel_func = std::bind(&ScriptRuntime::move_rel, this, std::placeholders::_1);
    this->methods.insert(std::pair<std::string, std::function<void(std::list<std::string>)>>("move_rel", move_rel_func));

    auto move_rel_look_func = std::bind(&ScriptRuntime::move_rel_look, this, std::placeholders::_1);
    this->methods.insert(std::pair<std::string, std::function<void(std::list<std::string>)>>("move_rel_look", move_rel_look_func));
}

void mcbot::ScriptRuntime::call_method(std::string method_name, std::list<std::string> args)
{
    this->methods.at(method_name)(args);
}

void mcbot::ScriptRuntime::move_rel(std::list<std::string> args)
{
    std::list<std::string>::iterator it = args.begin();

    Vector<double> vector = get_vector(*(it++));

    //this->bot->get_packet_sender().send_position(this->bot.get_player().get_location() + vector, true);
}

void mcbot::ScriptRuntime::move_rel_look(std::list<std::string> args)
{
    std::list<std::string>::iterator it = args.begin();

    Vector<double> vector = get_vector(*(it++));
    float yaw = std::stof(*(it++));
    float pitch = std::stof(*(it++));

    //this->bot.get_packet_sender().send_position_look(this->bot.get_player().get_location() + vector, yaw, pitch, true);
}
