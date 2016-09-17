#include "etj_player_queries.h"
#include "g_local.h"


std::string ETJump::Player::name(int clientNum) const
{
	auto ent = g_entities + clientNum;
	if (ent->client->pers.connected == ClientConnected::Disconnected)
	{
		return "";
	}
}
