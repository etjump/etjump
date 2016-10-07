#include "etj_cvar_manager.h"
#include "etj_local.h"

std::string ETJump::CvarManager::getString(const std::string& cvar)
{
	char buffer[MAX_CVAR_VALUE_STRING]{};
	trap_Cvar_VariableStringBuffer(cvar.c_str(), buffer, sizeof(buffer));
	return buffer;
}

int ETJump::CvarManager::getInteger(const std::string& cvar)
{
	return trap_Cvar_VariableIntegerValue(cvar.c_str());
}
