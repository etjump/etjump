#pragma once
#include "etj_iplayer_queries.h"

namespace ETJump
{
	/**
	 * A wrapper for player related
	 */
	class PlayerQueries : public IPlayerQueries
	{
	public:
		int connectedPlayerCount() const override;
		int playingPlayerCount() const override;
		std::string name(int clientNum) const override;
		Team team(int clientNum) const override;
	};
}



