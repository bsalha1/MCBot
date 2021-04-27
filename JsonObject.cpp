#include "JsonObject.h"

mcbot::JsonObject::JsonObject(std::string name)
{
    this->name = name;
}

std::string mcbot::JsonObject::extract_string(std::string json, std::size_t& offset)
{
    std::size_t first_quote_index = json.find('\"', offset);
    if (first_quote_index == std::string::npos)
    {
        return "";
    }
    offset = first_quote_index + 1;

    std::size_t second_quote_index = json.find('\"', offset);
    if (second_quote_index == std::string::npos)
    {
        return "";
    }
    offset = second_quote_index + 1;

    return json.substr(first_quote_index + 1, second_quote_index - first_quote_index - 1);
}

mcbot::JsonObject mcbot::JsonObject::extract_object(std::string json_dump, std::string name, std::size_t& index)
{
    mcbot::JsonObject child_object(name);
    std::string value;
    do
    {
        index++;
        name = extract_string(json_dump, index);
        value = extract_string(json_dump, index);
        child_object.add_pair(name, value);
    } while (json_dump[index] == ',');

    index++;
    return child_object;
}

std::list<mcbot::JsonObject> mcbot::JsonObject::extract_list(std::string json_dump, std::size_t& index)
{
    std::list<mcbot::JsonObject> object_list;

    do
    {
        mcbot::JsonObject child_object = extract_object(json_dump, "", index);
        object_list.push_back(child_object);
    } while (json_dump[index] == ',');

    index++;
    return object_list;
}

mcbot::JsonObject mcbot::JsonObject::serialize(std::string& json_dump)
{
    std::size_t index = 0;
    if (json_dump[index] != '{')
    {
        throw std::exception("invalid json format");
    }

    mcbot::JsonObject parent_object("");
    std::string name;
    std::string value;
    while (json_dump[index] != '}')
    {
        name = extract_string(json_dump, index);
        index++;

        if (json_dump[index] == '}')
        {
            break;
        }

        if (json_dump[index] == '{')
        {
            mcbot::JsonObject child_object = extract_object(json_dump, name, index);
            parent_object.add_pair(child_object.get_name(), child_object);
        }
        else if (json_dump[index] == '[')
        {
            std::list<mcbot::JsonObject> object_list = extract_list(json_dump, index);
            parent_object.add_pair(name, object_list);
        }
        else
        {
            value = extract_string(json_dump, index);
            parent_object.add_pair(name, value);
        }
    }

    return parent_object;
}

void mcbot::JsonObject::add_pair(std::string key, std::string value)
{
	this->raws_map.insert(std::pair<std::string, std::string>(key, value));
}

void mcbot::JsonObject::add_pair(std::string key, mcbot::JsonObject value)
{
	this->objects_map.insert(std::pair<std::string, mcbot::JsonObject>(key, value));
}

void mcbot::JsonObject::add_pair(std::string key, std::list<mcbot::JsonObject> value)
{
	this->lists_map.insert(std::pair<std::string, std::list<mcbot::JsonObject>>(key, value));
}

std::string mcbot::JsonObject::get_string(std::string key)
{
	return this->raws_map.find(key)->second;
}

mcbot::JsonObject mcbot::JsonObject::get_object(std::string key)
{
	return this->objects_map.find(key)->second;
}

std::list<mcbot::JsonObject> mcbot::JsonObject::get_list(std::string key)
{
	return this->lists_map.find(key)->second;
}

std::string mcbot::JsonObject::get_name()
{
	return this->name;
}

std::map<std::string, std::string> mcbot::JsonObject::get_raws_map()
{
	return this->raws_map;
}

std::map<std::string, mcbot::JsonObject> mcbot::JsonObject::get_objects_map()
{
	return this->objects_map;
}

std::map<std::string, std::list<mcbot::JsonObject>> mcbot::JsonObject::get_lists_map()
{
	return this->lists_map;
}

void insert_spaces(std::ostream& os, int num_spaces)
{
    for (int i = 0; i < num_spaces; i++)
    {
        os << " ";
    }
}

std::ostream& mcbot::operator<<(std::ostream& os, JsonObject obj)
{
    insert_spaces(os, 2 * obj.pretty_pointer);
    if (obj.name.length() == 0)
    {
        os << '{' << std::endl;
    }
    else
    {
        os << "\"" << obj.name << "\": {" << std::endl;
    }

    // Print raw key-values
    obj.pretty_pointer++;
    for (auto entry : obj.raws_map)
    {
        insert_spaces(os, 2 * obj.pretty_pointer);
        os << "\"" << entry.first << "\": \"" << entry.second << "\"," << std::endl;
    }
    obj.pretty_pointer--;


    // Print objects
    obj.pretty_pointer++;
    for (auto entry : obj.objects_map)
    {
        os << entry.second << ',' << std::endl;
    }
    obj.pretty_pointer--;


    // Print arrays
    obj.pretty_pointer++;
    for (auto entry : obj.lists_map)
    {
        insert_spaces(os, 2 * obj.pretty_pointer);
        os << "\"" << entry.first << "\": [" << std::endl;

        obj.pretty_pointer++;
        for (auto element : entry.second)
        {
            os << element << ',' << std::endl;
        }
        obj.pretty_pointer--;

        insert_spaces(os, 2 * obj.pretty_pointer);
        os << ']' << std::endl;
    }
    obj.pretty_pointer--;

    insert_spaces(os, 2 * obj.pretty_pointer);
    os << '}';

    return os;
}
