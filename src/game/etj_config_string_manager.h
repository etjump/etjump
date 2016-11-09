#pragma once
#include <string>
#include <functional>

namespace ETJump
{
	class ConfigStringManager
	{
	public:
		static const int MAX_CONFIG_STRING_LENGTH = 1024;

		ConfigStringManager(
			std::function<void(int configString, char *buffer, int bufferSize)> getConfigStringSyscall, 
			std::function<void(int configString, const char *str)> setConfigStringSyscall
		);
		~ConfigStringManager();

		// Returns a config string value as a string
		std::string get(int configString) const;

		// Returns a config string value as an integer
		// Throws std::invalid_argument if value is not an integer
		// Throws std::out_of_range if value is too large
		int getInt(int configString) const;

		// sets the config string value
		void set(int configString, const std::string& value) const;
		void set(int configString, int value) const;
	private:
		const std::function<void(int configString, char *buffer, int bufferSize)> _getConfigStringSyscall;
		const std::function<void(int configString, const char *str)> _setConfigStringSyscall;
	};
}



