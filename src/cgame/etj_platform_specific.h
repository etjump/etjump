#pragma once
#include <string>

namespace ETJump
{
	// General platform specific code
	class PlatformSpecific
	{
	public:
		PlatformSpecific();
		~PlatformSpecific();

		// minimizes the game
		static void minimize();

		// gets the hardware id
		static std::string hardwareId();
	};
}



