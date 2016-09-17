#pragma once
#include "etj_iplayer_queries.h"

namespace ETJump
{
	/**
	 * A wrapper for player related
	 */
	class Player : public IPlayerQueries
	{
	public:
		std::string name(int clientNum) const override;
	};
}



