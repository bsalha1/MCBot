#pragma once

#include <iostream>

namespace McBot
{
	class Logger
	{
	private:
		bool debug;

	public:
		Logger();

		void SetDebug(bool debug);

		void LogDebug(std::string message);
		void LogError(std::string message);
		void LogInfo(std::string message);
		void LogChat(std::string message);
	};
}

