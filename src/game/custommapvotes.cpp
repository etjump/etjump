#include "custommapvotes.hpp"
#include <fstream>
#include "g_utilities.hpp"
#include "../json/json.h"

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
    std::string path = GetPath("customvotes.json");
    std::ifstream f(path.c_str());

    if (!f)
    {
        G_Error("Couldn't open customvotes.json");
        return false;
    }
    std::string content((std::istreambuf_iterator<char>(f)),
        std::istreambuf_iterator<char>());
    
    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(content, root))
    {
        G_Error("parser error");
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