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
		MCBot& bot;

		std::map<std::string, std::function<void(std::list<std::string>)>> methods;

	public:
		ScriptRuntime(MCBot& bot);

		void CallMethod(std::string method_name, std::list<std::string> args);

		void MoveRel(std::list<std::string> args);

		void MoveRelLook(std::list<std::string> args);
	};
}

