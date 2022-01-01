/*
 * MIT License
 * 
 * Copyright (c) 2021 ETJump team <zero@etjump.com>
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

#include "etj_custom_map_votes.h"
#include "etj_map_statistics.h"
#include <fstream>
#include "utilities.hpp"
#include "../json/json.h"
#include <algorithm>

CustomMapVotes::CustomMapVotes(MapStatistics *mapStats) : _mapStats(mapStats)
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
	_currentMapsOnServer = _mapStats->getCurrentMaps();

	customMapVotes_.clear();
	std::string   path = GetPath(g_customMapVotesFile.string);
	std::ifstream f(path.c_str());

	if (!f)
	{
		G_LogPrintf("Couldn't open \"%s\". Use /rcon generatecustomvotes to generate an example file.\n", g_customMapVotesFile.string);
		return false;
	}
	std::string content((std::istreambuf_iterator<char>(f)),
	                    std::istreambuf_iterator<char>());

	Json::Value  root;
	Json::Reader reader;

	if (!reader.parse(content, root))
	{
		G_LogPrintf("There was a parsing error in the %s: %s\n", reader.getFormattedErrorMessages().c_str(), g_customMapVotesFile.string);
		return false;
	}

	try
	{
		for (int i = 0; i < static_cast<int>(root.size()); i++)
		{
			customMapVotes_.push_back(MapType());
			unsigned    curr = customMapVotes_.size() - 1;
			Json::Value maps = root[i]["maps"];
			customMapVotes_[curr].type         = root[i]["name"].asString();
			customMapVotes_[curr].callvoteText = root[i]["callvote_text"].asString();
			for (int j = 0; j < static_cast<int>(maps.size()); j++)
			{
				auto mapName = maps[j].asString();
				if (std::binary_search(_currentMapsOnServer->begin(), _currentMapsOnServer->end(), mapName))
				{
					customMapVotes_[curr].mapsOnServer.push_back(mapName);
				}
				else
				{
					customMapVotes_[curr].otherMaps.push_back(mapName);
				}
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
	Arguments     argv = GetArgs();
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
	vote["name"]          = "originals";
	vote["callvote_text"] = "Original 6 Maps";
	vote["maps"]          = Json::arrayValue;
	vote["maps"].append("oasis");
	vote["maps"].append("fueldump");
	vote["maps"].append("radar");
	vote["maps"].append("goldrush");
	vote["maps"].append("railgun");
	vote["maps"].append("battery");
	root.append(vote);
	vote["name"]          = "just_oasis";
	vote["callvote_text"] = "Just oasis";
	vote["maps"]          = Json::arrayValue;
	vote["maps"].append("oasis");
	root.append(vote);

	Json::StyledWriter writer;
	std::string        output = writer.write(root);
	std::ofstream      fOut(GetPath(g_customMapVotesFile.string).c_str());
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

			int count = 0;
			for (auto & mapOnServer : customMapVotes_[i].mapsOnServer)
			{
				lines.push_back(va("^7%-30s ", mapOnServer.c_str()));

				++count;
				if (count % 3)
				{
					lines.push_back("\n");
				}
			}
			if (lines[lines.size() - 1] != "\n")
			{
				lines.push_back("\n");
			}
			lines.push_back("^<Maps that are not on the server: ^7\n");
			count = 0;
			for (auto & mapNotOnServer : customMapVotes_[i].otherMaps)
			{
				lines.push_back(va("^9%-30s ", mapNotOnServer.c_str()));

				++count;
				if (count % 3)
				{
					lines.push_back("\n");
				}
			}
			if (lines[lines.size() - 1] != "\n")
			{
				lines.push_back("\n");
			}
		}
	}

	return &lines;
}

std::string const CustomMapVotes::RandomMap(std::string const& type)
{
	for (unsigned i = 0; i < customMapVotes_.size(); i++)
	{
		auto &customMapVote = customMapVotes_[i];
		if (customMapVote.type == type && customMapVote.mapsOnServer.size() > 0)
		{
			const int MAX_TRIES = 15;
			std::random_device                 rd;
			std::mt19937                       re(rd());
			std::uniform_int_distribution<int> ui(0, customMapVote.mapsOnServer.size() - 1);

			// try to select a valid random map
			for (int tries = 0; tries < MAX_TRIES; tries++)
			{
				int testIdx = ui(re);
				if (isValidMap(customMapVote.mapsOnServer[testIdx]))
				{
					return customMapVote.mapsOnServer[testIdx];
				}
			}
			// fallback, iterate map list and select first valid map
			for (int i = 0; i < static_cast<int>(customMapVote.mapsOnServer.size()); i++)
			{
				if (isValidMap(customMapVote.mapsOnServer[i]))
				{
					return customMapVote.mapsOnServer[i];
				}
			}

			return "";
		}
	}
	return "";
}

bool CustomMapVotes::isValidMap(const std::string &mapName)
{
	MapStatistics mapStats;

	return	G_MapExists(mapName.c_str()) && 
			mapName != level.rawmapname &&
			strstr(mapStats.getBlockedMapsStr().c_str(), mapName.c_str()) == nullptr;
}
