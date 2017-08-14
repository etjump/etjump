#include "etj_local.h"
#include "etj_game.h"
#include "etj_save.h"
#include "etj_custom_map_votes.h"
#include "etj_utilities.h"
#include <boost/algorithm/string.hpp>
#include "etj_motd.h"
#include "etj_timerun.h"
#include "etj_map_statistics.h"
#include "etj_tokens.h"
#include "etj_shared.h"
#include "etj_session_service.h"
#include "utilities.hpp"

Game game;

void RunFrame(int levelTime)
{
	game.mapStatistics->runFrame(levelTime);
}

void OnGameInit()
{
	game.saves          = std::make_shared<SaveSystem>();
	game.mapStatistics  = std::make_shared<MapStatistics>();
	game.customMapVotes = std::make_shared<CustomMapVotes>(game.mapStatistics.get());
	game.motd           = std::make_shared<Motd>();
	game.timerun        = std::make_shared<Timerun>();
	game.tokens         = std::make_shared<Tokens>();
	game.mapStatistics->initialize(std::string(g_mapDatabase.string), level.rawmapname);
	game.customMapVotes->Load();
	game.motd->Initialize();
	game.timerun->init(GetPath(g_timerunsDatabase.string), level.rawmapname);

	if (g_tokensMode.integer)
	{
		// Utilities::WriteFile handles the correct path (etjump/...)
		auto path = std::string(g_tokensPath.string) + "/" + std::string(level.rawmapname) + ".json";
		game.tokens->loadTokens(path);
	}
}

void OnGameShutdown()
{
	game.mapStatistics->saveChanges();
	game.tokens->reset();

	game.saves = nullptr;
	game.customMapVotes = nullptr;
	game.motd = nullptr;
	game.timerun = nullptr;
	game.mapStatistics = nullptr;
	game.tokens = nullptr;
}

/*
=======================
Server console commands
=======================
*/
qboolean OnConsoleCommand()
{
	G_DPrintf("OnConsoleCommand called: %s.\n", ConcatArgs(0));

	auto argv    = GetArgs();
	auto command = (*argv)[0];
	boost::to_lower(command);

	if (command == "generatemotd")
	{
		game.motd->GenerateMotdFile();
		return qtrue;
	}

	if (command == "generatecustomvotes")
	{
		game.customMapVotes->GenerateVotesFile();
		return qtrue;
	}

	if (command == "logstate")
	{
		LogServerState();
		return qtrue;
	}

	return qfalse;
}

const char *GetRandomMap()
{
	return game.mapStatistics->randomMap();
}

const char *G_MatchOneMap(const char *arg)
{
	auto                     currentMaps = game.mapStatistics->getCurrentMaps();
	std::vector<std::string> matchingMaps;
	std::string              mapName = arg ? arg : "";
	boost::algorithm::to_lower(mapName);

	for (auto & map : *(currentMaps))
	{
		if (map.find(mapName) != std::string::npos)
		{
			matchingMaps.push_back(map);
		}
	}

	static char matchingMap[MAX_STRING_TOKENS] = "\0";
	if (matchingMaps.size() == 1)
	{
		Q_strncpyz(matchingMap, matchingMaps[0].c_str(), sizeof(matchingMap));
		return matchingMap;
	}

	if (matchingMaps.size() > 1)
	{
		auto match = std::find(matchingMaps.begin(), matchingMaps.end(), mapName);
		if (match != matchingMaps.end())
		{
			Q_strncpyz(matchingMap, match->c_str(), sizeof(matchingMap));
			return matchingMap;
		}
	}

	return NULL;
}

const char *GetRandomMapByType(const char *customType)
{
	static char buf[MAX_TOKEN_CHARS] = "\0";

	if (!customType)
	{
		G_Error("customType is NULL.");
	}

	Q_strncpyz(buf, game.customMapVotes->RandomMap(customType).c_str(), sizeof(buf));
	return buf;
}

// returns null if map type doesnt exists.
const char *CustomMapTypeExists(const char *mapType)
{
	static char buf[MAX_TOKEN_CHARS] = "\0";
	if (!mapType)
	{
		G_Error("mapType is NULL.");
	}

	CustomMapVotes::TypeInfo info = game.customMapVotes->GetTypeInfo(mapType);

	if (info.typeExists)
	{
		Q_strncpyz(buf, info.callvoteText.c_str(), sizeof(buf));
		return buf;
	}

	return NULL;
}

std::string GetTeamString(int clientNum)
{
	gentity_t *ent = g_entities + clientNum;

	if (ent->client->sess.sessionTeam == TEAM_ALLIES)
	{
		return "ALLIES";
	}
	if (ent->client->sess.sessionTeam == TEAM_AXIS)
	{
		return "AXIS";
	}
	return "SPECTATOR";
}

void LogServerState()
{
	time_t t;
	time(&t);
	std::string   state = "Server state at " + TimeStampToString(static_cast<int>(t)) + ":\n";
	boost::format fmter("%1% %2% %3%");
	for (int i = 0; i < level.numConnectedClients; i++)
	{
		int clientNum = level.sortedClients[i];

		fmter % clientNum % GetTeamString(clientNum) % (g_entities + clientNum)->client->pers.netname;
		state += fmter.str() + "\n";
	}

	if (level.numConnectedClients == 0)
	{
		state += "No players on the server.\n";
	}

	LogPrint(state);
}

void StartTimer(const char *runName, gentity_t *ent)
{
	game.timerun->startTimer(runName, ClientNum(ent), ent->client->pers.netname, ent->client->ps.commandTime);
}
void StopTimer(const char *runName, gentity_t *ent)
{
	game.timerun->stopTimer(ClientNum(ent), ent->client->ps.commandTime, runName);
}
void TimerunConnectNotify(gentity_t *ent)
{
	game.timerun->connectNotify(ClientNum(ent));
}
void InterruptRun(gentity_t *ent)
{
	if (!ent)
	{
		return;
	}

	game.timerun->interrupt(ClientNum(ent));
}

void G_increaseCallvoteCount(const char *mapName)
{
	game.mapStatistics->increaseCallvoteCount(mapName);
}

void G_increasePassedCount(const char *mapName)
{
	game.mapStatistics->increasePassedCount(mapName);
}

bool allTokensCollected(gentity_t *ent)
{
	auto tokenCounts = game.tokens->getTokenCounts();

	auto easyCount   = 0;
	auto mediumCount = 0;
	auto hardCount   = 0;
	for (auto i = 0; i < MAX_TOKENS_PER_DIFFICULTY; ++i)
	{
		if (ent->client->pers.collectedEasyTokens[i])
		{
			++easyCount;
		}

		if (ent->client->pers.collectedMediumTokens[i])
		{
			++mediumCount;
		}

		if (ent->client->pers.collectedHardTokens[i])
		{
			++hardCount;
		}
	}

	return tokenCounts[0] == easyCount && tokenCounts[1] == mediumCount && tokenCounts[2] == hardCount;
}
