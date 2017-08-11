#include "etj_config_parser.h"
#include <iterator>
#include "etj_parser.h"
bool Q_StartsAndEndsWith(const char *text, const char *start, const char *end);

ETJump::ConfigParser::ConfigParser(const std::string& config): _configParsed(false), _config(config)
{
}

ETJump::ConfigParser::~ConfigParser()
{
}


std::vector<ETJump::ConfigEntry> ETJump::ConfigParser::getEntries()
{
	if (!_configParsed)
	{
		//_entries = parseEntries(_config);
		_configParsed = true;
	}

	return _entries;
}
