#pragma once

#include <iostream>
#include <map>
#include <list>

namespace mcbot
{
	class JsonObject
	{
	private:
		std::string name;
		std::map<std::string, std::string> raws_map;
		std::map<std::string, std::list<mcbot::JsonObject>> lists_map;
		std::map<std::string, mcbot::JsonObject> objects_map;

		static std::string extract_string(std::string json, std::size_t& offset);
		static mcbot::JsonObject extract_object(std::string json, std::string name, std::size_t& index);
		static std::list<mcbot::JsonObject> extract_list(std::string json, std::size_t& index);

	public:
		JsonObject(std::string name);

		static mcbot::JsonObject serialize(std::string& json_dump);

		void add_pair(std::string key, std::string value);

		void add_pair(std::string key, mcbot::JsonObject value);

		void add_pair(std::string key, std::list<mcbot::JsonObject> value);

		std::string get_string(std::string key);

		mcbot::JsonObject get_object(std::string key);

		std::list<mcbot::JsonObject> get_list(std::string key);

		std::string get_name();

		std::map<std::string, std::string> get_raws_map();

		std::map<std::string, mcbot::JsonObject> get_objects_map();

		std::map<std::string, std::list<mcbot::JsonObject>> get_lists_map();

		friend std::ostream& operator<<(std::ostream& os, const JsonObject& obj);
	};
}
