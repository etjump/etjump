#pragma once
#include <string>
#include <functional>
#include "etj_iconfig_string_facade.h"

namespace ETJump
{
	class ConfigStringFacade : public IConfigStringFacade
	{
	public:

		ConfigStringFacade(
			std::function<void(int configString, char *buffer, int bufferSize)> getConfigStringSyscall, 
			std::function<void(int configString, const char *str)> setConfigStringSyscall
		);
		~ConfigStringFacade();

		// Returns a config string value as a string
		std::string get(int configString) const override;

		// Returns a config string value as an integer
		// Throws std::invalid_argument if value is not an integer
		// Throws std::out_of_range if value is too large
		int getInt(int configString) const override;

		// sets the config string value
		void set(int configString, const std::string& value) const override;
		void set(int configString, int value) const override;
	private:
		const std::function<void(int configString, char *buffer, int bufferSize)> _getConfigStringSyscall;
		const std::function<void(int configString, const char *str)> _setConfigStringSyscall;
	};
}



