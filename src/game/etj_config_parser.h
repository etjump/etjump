#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace ETJump
{
	struct ConfigEntry
	{
		std::string name;
		std::map<std::string, std::string> values;
	};

	class ConfigParser
	{
	public: 
		explicit ConfigParser(const std::string& config);
		~ConfigParser();

		std::vector<ConfigEntry> getEntries();
	private:

		std::string _config;
		bool _configParsed;
		std::vector<ConfigEntry> _entries;
	};
}



