#pragma once
#include <string>
#include "../game/q_shared.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace ETJump
{
	typedef std::initializer_list<const char*> ShaderStage;
	typedef std::initializer_list<ShaderStage> ShaderStages;
	std::string composeShader(const char *name, ShaderStage general, ShaderStages stages);
	std::string composeShader(const char *name, ShaderStages stages);
	void parseColorString(const std::string &colorString, vec4_t &color);
}
