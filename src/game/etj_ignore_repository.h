#pragma once
#include "etj_iignore_repository.h"

namespace ETJump
{
	class IgnoreRepository : public IIgnoreRepository
	{
	public:
		IgnoreRepository();
		~IgnoreRepository();
		void add(const std::string& forPlayerGuid, const std::string& targetPlayerGuid, const std::string& targetPlayerName) override;
		void remove(const std::string& forPlayerGuid, int ignoreId) override;
		std::vector<Ignore> getIgnoredPlayers(const std::string& playerGuid) override;
	};
}



