#pragma once
#include <string>
#include <vector>
#include "etj_ignore.h"

namespace ETJump
{
	class IIgnoreService
	{
	public:
		virtual ~IIgnoreService() = default;
		// Adds ignored player to calling player's ignore list => no messages from ignored player will be displayed 
		// to the calling player
		virtual void ignore(const std::string& callingPlayerGuid, const std::string& ignoredPlayerGuid, const std::string& ignoredPlayerName) = 0;
		// Removes the ignore
		virtual void unignore(const std::string& callingPlayerGuid, int ignoreId) = 0;
		// Lists ignored players
		virtual std::vector<Ignore> getIgnoredPlayers(const std::string& playerGuid) = 0;
		// Checks if other player is ignored for current player
		virtual bool isIgnored(const std::string& callingPlayerGuid, const std::string& ignoredPlayerGuid) = 0;
	};
}
