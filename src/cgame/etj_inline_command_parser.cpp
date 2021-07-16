/*
 * MIT License
 * 
 * Copyright (c) 2020 ETJump team <zero@etjump.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
    // bool lastWasSeparator = false;
    for (const auto &arg : args)
    {
        if (isSeparator(arg))
        {
            // lastWasSeparator = true;
            if (currentCommand.length() > 0)
            {
                commands.push_back(currentCommand);
                currentCommand = "";
            }
        } 
        else
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
