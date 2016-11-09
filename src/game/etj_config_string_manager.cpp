#include "etj_config_string_manager.h"


ETJump::ConfigStringManager::ConfigStringManager(std::function<void(int configString, char* buffer, int bufferSize)> getConfigStringSyscall, std::function<void(int configString, const char* str)> setConfigStringSyscall)
	: _getConfigStringSyscall(getConfigStringSyscall), _setConfigStringSyscall(setConfigStringSyscall)
{
}

ETJump::ConfigStringManager::~ConfigStringManager()
{
}

std::string ETJump::ConfigStringManager::get(int configString) const
{
	char buffer[MAX_CONFIG_STRING_LENGTH] = "";
	_getConfigStringSyscall(configString, buffer, sizeof(buffer));
	buffer[MAX_CONFIG_STRING_LENGTH - 1] = 0;
	return buffer;
}

int ETJump::ConfigStringManager::getInt(int configString) const
{
	return std::stoi(get(configString));
}

void ETJump::ConfigStringManager::set(int configString, const std::string& value) const
{
	_setConfigStringSyscall(configString, value.c_str());
}

void ETJump::ConfigStringManager::set(int configString, int value) const
{
	_setConfigStringSyscall(configString, std::to_string(value).c_str());
}

