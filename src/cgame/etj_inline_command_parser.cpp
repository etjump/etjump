#include "etj_inline_command_parser.h"
#include "../game/etj_string_utilities.h"

ETJump::InlineCommandParser::InlineCommandParser()
{
}

ETJump::InlineCommandParser::~InlineCommandParser()
{
}

std::vector<std::string> ETJump::InlineCommandParser::parse(const std::vector<std::string>& args)
{
    std::vector<std::string> commands;

    std::string currentCommand;
    bool lastWasSeparator = false;
    for (auto arg : args)
    {
        if (isSeparator(arg))
        {
            lastWasSeparator = true;
            if (currentCommand.length() > 0)
            {
                commands.push_back(currentCommand);
                currentCommand = "";
            }
        } else
        {
            currentCommand += "\"" + arg + "\"";
        }
    }

    if (currentCommand.length() > 0)
    {
        commands.push_back(currentCommand);
    }

    return commands;
}

bool ETJump::InlineCommandParser::isSeparator(const std::string& input)
{
    return trim(input) == "|";
}
