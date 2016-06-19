#pragma once
#include <string>

namespace ETJump
{
	class ConsoleVariables
	{
	public:
		// returns the variable with the specified name
		// returns an empty string if no match was found
		static std::string getCvarString(const std::string& name);
		// returns the variable with the specified name
		// returns 0 if no match was found
		static int getCvarInteger(const std::string& name);
		//// returns the variable with the specified 
		//// returns 0 if no match was found
		//static float getCvarFloat(const std::string& name);
	};
}



