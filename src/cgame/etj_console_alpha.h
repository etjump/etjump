#pragma once

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace ETJump
{
	class ConsoleAlphaHandler
	{
		const char *shaderName{ "__etjump-console-shader__" };
		std::string createBackground();
		std::string createTexturedBackground();
		std::string createSolidBackground();
	public:
		ConsoleAlphaHandler();
		~ConsoleAlphaHandler();
	};
}
