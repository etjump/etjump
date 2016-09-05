#pragma once
#include <string>

namespace ETJump
{
	class ConsoleVariables
	{
	public:
		static std::string getString(const std::string& cvar);
		static int getInt(const std::string& cvar);
		static float getFloat(const std::string& cvar);
	};
}



