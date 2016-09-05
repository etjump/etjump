#include "etj_console_variables.h"

#include "g_local.h"

std::string ETJump::ConsoleVariables::getString(const std::string& cvar)
{
	char buf[MAX_CVAR_VALUE_STRING] = "";
	trap_Cvar_VariableStringBuffer(cvar.c_str(), buf, sizeof(buf));
	return buf;
}

int ETJump::ConsoleVariables::getInt(const std::string& cvar)
{
	return trap_Cvar_VariableIntegerValue(cvar.c_str());
}

float ETJump::ConsoleVariables::getFloat(const std::string& cvar)
{
	return trap_Cvar_VariableIntegerValue(cvar.c_str());
}
