#include "etj_player_queries.h"
#include "g_local.h"


int ETJump::PlayerQueries::connectedPlayerCount() const
{
	return level.numConnectedClients;
}

int ETJump::PlayerQueries::playingPlayerCount() const
{
	auto count = 0;
	for (auto i = 0; i < level.numConnectedClients; ++i)
	{
		auto client = (g_entities + level.sortedClients[i])->client;
		if (client->sess.sessionTeam == TEAM_AXIS || client->sess.sessionTeam == TEAM_ALLIES)
		{
			++count;
		}
	}
	return count;
}

std::string ETJump::PlayerQueries::name(int clientNum) const
{
	if (clientNum < 0 || clientNum >= MAX_CLIENTS) return "^7Server";

	auto ent = g_entities + clientNum;
	if (ent->client->pers.connected == ClientConnected::Disconnected)
	{
		return "";
	}

	return ent->client->pers.netname;
}

ETJump::Team ETJump::PlayerQueries::team(int clientNum) const
{
	if (clientNum < 0 || clientNum >= MAX_CLIENTS) return Team::Free;

	auto ent = g_entities + clientNum;
	switch (ent->client->sess.sessionTeam)
	{
	case TEAM_AXIS: 
		return Team::Axis;
	case TEAM_ALLIES: 
		return Team::Allies;
	case TEAM_SPECTATOR: 
		return Team::Spectator;
	default: 
		return Team::Free;
	}
}
