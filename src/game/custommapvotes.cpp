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
    if (strlen(g_customMapVotesFile.string) == 0)
    {
        return false;
    }
    customMapVotes_.clear();
    std::string path = GetPath(g_customMapVotesFile.string);
    std::ifstream f(path.c_str());

    if (!f)
    {
        G_LogPrintf("Couldn't open \"%s\". Use /rcon generatecustomvotes to generate an example file.\n", g_customMapVotesFile.string);
        return false;
    }
    std::string content((std::istreambuf_iterator<char>(f)),
        std::istreambuf_iterator<char>());
    
    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(content, root))
    {
        G_LogPrintf("There was a parsing error in the %s: %s\n", reader.getFormattedErrorMessages().c_str(), g_customMapVotesFile.string);
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
        G_LogPrintf("There was a read error in %s parser\n", g_customMapVotesFile.string);
        return false;
    }
    

    G_LogPrintf("Successfully initialized custom votes.\n");
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

void CustomMapVotes::GenerateVotesFile()
{
    Arguments argv = GetArgs();
    std::ifstream in(GetPath(g_customMapVotesFile.string).c_str());
    if (in.good())
    {
        if (argv->size() == 1)
        {
            G_Printf("A custom map votes file exists. Are you sure you want to overwrite the write? Do /rcon generatecustomvotes -f if you are sure.");
            return;
        }
        if (argv->size() == 2 && argv->at(1) == "-f")
        {
            G_LogPrintf("Overwriting custom map votes file with a default one.\n");
        }
        else
        {
            G_Printf("Unknown argument \"%s\".\n", argv->at(1).c_str());
            return;
        }
        
    }
    in.close();
    Json::Value root = Json::arrayValue;
    Json::Value vote;
    vote["name"] = "originals";
    vote["callvote_text"] = "Original 6 Maps";
    vote["maps"] = Json::arrayValue;
    vote["maps"].append("oasis");
    vote["maps"].append("fueldump");
    vote["maps"].append("radar");
    vote["maps"].append("goldrush");
    vote["maps"].append("railgun");
    vote["maps"].append("battery");
    root.append(vote);
    vote["name"] = "just_oasis";
    vote["callvote_text"] = "Just oasis";
    vote["maps"] = Json::arrayValue;
    vote["maps"].append("oasis");
    root.append(vote);

    Json::StyledWriter writer;
    std::string output = writer.write(root);
    std::ofstream fOut(GetPath(g_customMapVotesFile.string).c_str());
    if (!fOut)
    {
        G_Printf("Couldn't open file \"%s\" defined in g_customMapVotesFile.\n", g_customMapVotesFile.string);
        return;
    }
    fOut << output;
    fOut.close();
    G_Printf("Generated new custom map votes file \"%s\"\n", g_customMapVotesFile.string);
    Load();
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