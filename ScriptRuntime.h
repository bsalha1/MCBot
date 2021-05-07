#pragma once

#include <iostream>
#include <map>
#include <string>
#include <functional>
#include <list>

#include "MCBot.h"

namespace mcbot
{
	class ScriptRuntime
	{
	private:
		mcbot::MCBot& bot;

		std::map<std::string, std::function<void(std::list<std::string>)>> methods;

	public:
		ScriptRuntime(mcbot::MCBot& bot);

		void call_method(std::string method_name, std::list<std::string> args);

		void move_rel(std::list<std::string> args);

		void move_rel_look(std::list<std::string> args);
	};
}

