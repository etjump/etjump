#include "custommapvotes.hpp"
#include <fstream>
#include "g_utilities.hpp"
#include "../json/json.h"
#include <boost/algorithm/string.hpp>

CustomMapVotes::CustomMapVotes()
{
}

CustomMapVotes::~CustomMapVotes()
{
}

CustomMapVotes::TypeInfo CustomMapVotes::GetTypeInfo(std::string const& type) const
{
    for (unsigned i = 0; i < customMapVotes_.size(); i++)
    {
        if (customMapVotes_[i].type == type)
        {
            return TypeInfo(true, customMapVotes_[i].callvoteText);
        }
    }
    return TypeInfo(false, "");
}

bool CustomMapVotes::Load()
{
    customMapVotes_.clear();
    std::string path = GetPath("customvotes.json");
    std::ifstream f(path.c_str());

    if (!f)
    {
        G_LogPrintf("Couldn't open customvotes.json");
        return false;
    }
    std::string content((std::istreambuf_iterator<char>(f)),
        std::istreambuf_iterator<char>());
    
    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(content, root))
    {
        G_Error("Customvotes parser error");
        return false;
    }

    try
    {
        for (int i = 0; i < root.size(); i++)
        {
            customMapVotes_.push_back(MapType());
            unsigned curr = customMapVotes_.size() - 1;
            Json::Value maps = root[i]["maps"];
            customMapVotes_[curr].type = root[i]["name"].asString();
            customMapVotes_[curr].callvoteText = root[i]["callvote_text"].asString();
            for (int j = 0; j < maps.size(); j++)
            {
                customMapVotes_[curr].maps.push_back(maps[j].asString());
            }
        }
    }
    catch (...)
    {
        G_Error("Failed to read a value from json");
        return false;
    }
    
    return true;
}

std::string CustomMapVotes::ListTypes() const
{
    std::string buf;
    for (unsigned i = 0; i < customMapVotes_.size(); i++)
    {
        if (i != (customMapVotes_.size() - 1))
        {
            buf += customMapVotes_[i].type + ", ";
        }
        else
        {
            buf += customMapVotes_[i].type;
        }
    }
    return buf;
}

const std::vector<std::string> *CustomMapVotes::ListInfo(const std::string& type)
{
    static std::vector<std::string> lines;
    lines.clear();
    
    for (unsigned i = 0; i < customMapVotes_.size(); i++)
    {
        if (customMapVotes_[i].type == type)
        {
            lines.push_back("^<Maps on the list: ^7\n");
            for (unsigned j = 0; j < customMapVotes_[i].maps.size(); j++)
            {
                lines.push_back(va("%-30s ", customMapVotes_[i].maps[j].c_str()));
                if (j % 3 == 0 && j != 0)
                {
                    lines[lines.size() - 1].push_back('\n');
                }
                else if (j + 1 == customMapVotes_[i].maps.size())
                {
                    lines[lines.size() - 1].push_back('\n');
                }
            }
        }
    }
    
    return &lines;
}

std::string const CustomMapVotes::RandomMap(std::string const& type)
{
    for (unsigned i = 0; i < customMapVotes_.size(); i++)
    {
        if (customMapVotes_[i].type == type)
        {
            if (customMapVotes_[i].maps.size() == 0)
            {
                return "";
            }
            // TODO: check if map exists
            return customMapVotes_[i].maps[rand() % customMapVotes_[i].maps.size()];
        }
    }
    return "";
}