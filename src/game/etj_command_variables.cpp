#include "etj_console_variables.h"

#include "g_local.h"

std::string ETJump::ConsoleVariables::getCvarString(const std::string& name)
{
	char buffer[MAX_CVAR_VALUE_STRING];
	trap_Cvar_VariableStringBuffer(name.c_str(), buffer, sizeof(buffer));
	return buffer;
}

int ETJump::ConsoleVariables::getCvarInteger(const std::string& name)
{
	return trap_Cvar_VariableIntegerValue(name.c_str());
}

//float ETJump::ConsoleVariables::getCvarFloat(const std::string& name)
//{
//	return trap_Cvar_VariableValue(name.c_str());
//}
