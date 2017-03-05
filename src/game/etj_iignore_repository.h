#pragma once

#include <vector>
#include <string>
#include "etj_ignore.h"

namespace ETJump
{
	class IIgnoreRepository
	{
	public:
		virtual ~IIgnoreRepository() = default;
		// adds the target player guid to the ignore list
		virtual void add(const std::string& forPlayerGuid, const std::string& targetPlayerGuid, const std::string& targetPlayerName) = 0;
		// removes the target player from the ignore list
		virtual void remove(const std::string& forPlayerGuid, int ignoreId) = 0;
		// gets target player's all ignored players
		virtual std::vector<Ignore> getIgnoredPlayers(const std::string& playerGuid) = 0;
	};
}

