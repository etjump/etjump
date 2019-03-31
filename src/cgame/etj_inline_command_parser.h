#pragma once
#include <vector>
#include <string>

namespace ETJump
{
    class InlineCommandParser
    {
    public:
        InlineCommandParser();
        ~InlineCommandParser();

        std::vector<std::string> parse(const std::vector<std::string>& args);
    private:
        bool isSeparator(const std::string& input);
    };
}
