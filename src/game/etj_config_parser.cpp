#include "etj_config_parser.h"
#include "q_shared.h"
#include <iterator>
#include <boost/format/free_funcs.hpp>
#include <boost/algorithm/string.hpp>
bool Q_StartsAndEndsWith(const char *text, const char *start, const char *end);

ETJump::ConfigParser::ConfigParser(const std::string& config): _configParsed(false)
{
	std::copy(begin(config), end(config), std::back_inserter(_config));
}

ETJump::ConfigParser::ConfigParser(const std::vector<char>& config): _config(config), _configParsed(false)
{
}

ETJump::ConfigParser::~ConfigParser()
{
}

std::string ETJump::ConfigParser::readString(char** current)
{
	auto token = COM_ParseExt(current, qfalse);

	if (!Q_stricmp(token, "="))
	{
		token = COM_ParseExt(current, qfalse);
	}
	else
	{
		throw std::runtime_error("readconfig: missing = before \"%s\" on line %d.");
	}
	std::string str;

	while (token[0])
	{
		str += token;
		str.push_back(' ');
		token = COM_ParseExt(current, qfalse);
	}

	boost::trim_right(str);
	return str;
}

std::vector<ETJump::ConfigEntry> ETJump::ConfigParser::parseEntries(std::vector<char>& config)
{
	std::vector<ETJump::ConfigEntry> entries;
	char *original = config.data();
	auto current = &original;

	COM_BeginParseSession("config_entries");

	auto token = COM_Parse(current);
	ConfigEntry currentEntry;
	while (*token)
	{
		if (Q_StartsAndEndsWith(token, "[", "]"))
		{
			if (currentEntry.name.length() > 0)
			{
				entries.push_back(currentEntry);
			} 

			std::string tokenStr = std::string(token);
			currentEntry.name = tokenStr.substr(1, tokenStr.length() - 2);
		} else
		{
			std::string name = token;
			
			try
			{
				std::string value = readString(current);
				currentEntry.values[name] = value;
			} catch (const std::runtime_error&)
			{
				// todo display some error?
			}
		}

		token = COM_Parse(current);
	}

	entries.push_back(currentEntry);

	return std::move(entries);
}


std::vector<ETJump::ConfigEntry> ETJump::ConfigParser::getEntries()
{
	if (!_configParsed)
	{
		_entries = parseEntries(_config);
		_configParsed = true;
	}

	return _entries;
}
