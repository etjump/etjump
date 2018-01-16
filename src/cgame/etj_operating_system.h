#pragma once
#include <string>

namespace ETJump
{
	class OperatingSystem
	{
	public:
		OperatingSystem();
		void minimize();
		std::string getHwid();
		void addMinimizeButton();
	};
}
