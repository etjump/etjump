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

	class ConfigSerializer
	{
	public: 
		explicit ConfigSerializer(const std::string& config);
		explicit ConfigSerializer(const std::vector<char>& config);
		~ConfigSerializer();


		std::string readString(char** current);
		std::vector<ConfigEntry> deserialize();
		static std::string serialize(const std::vector<ConfigEntry>& entries);
		std::vector<std::string> getErrors();
	private:
		std::vector<ConfigEntry> deserialize(std::vector<char>& config);
		std::vector<char> _config;
		bool _configParsed;
		std::vector<ConfigEntry> _entries;
		std::vector<std::string> _errors;
	};
}



