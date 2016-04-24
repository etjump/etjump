#include "etj_command_variables.hpp"

extern "C" {
#include "g_local.h"
}


std::string ETJump::CommandVariables::getCvarString(const std::string& name)
{
	char buffer[MAX_CVAR_VALUE_STRING];
	trap_Cvar_VariableStringBuffer(name.c_str(), buffer, sizeof(buffer));
	return buffer;
}

int ETJump::CommandVariables::getCvarInteger(const std::string& name)
{
	return trap_Cvar_VariableIntegerValue(name.c_str());
}

//float ETJump::CommandVariables::getCvarFloat(const std::string& name)
//{
//	return trap_Cvar_VariableValue(name.c_str());
//}
