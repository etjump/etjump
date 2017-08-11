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
		explicit ConfigParser(const std::vector<char>& config);
		~ConfigParser();


		std::string readString(char** current);
		std::vector<ConfigEntry> getEntries();
	private:
		std::vector<ConfigEntry> parseEntries(std::vector<char>& config);
		std::vector<char> _config;
		bool _configParsed;
		std::vector<ConfigEntry> _entries;
	};
}



