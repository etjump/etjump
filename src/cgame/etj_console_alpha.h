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
	public:
		ConsoleAlphaHandler();
		~ConsoleAlphaHandler();
	};
}
