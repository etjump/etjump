#pragma once
#include <string>

namespace ETJump
{
	class IConfigStringFacade
	{
	public:
		virtual ~IConfigStringFacade()
		{
		}

		static const int MAX_CONFIG_STRING_LENGTH = 1024;
		
		// Returns a config string value as a string
		virtual std::string get(int configString) const = 0;

		// Returns a config string value as an integer
		// Throws std::invalid_argument if value is not an integer
		// Throws std::out_of_range if value is too large
		virtual int getInt(int configString) const = 0;

		// sets the config string value
		virtual void set(int configString, const std::string& value) const = 0;
		virtual void set(int configString, int value) const = 0;
	};
}
