#pragma once

#include "etj_iignore_service.h"
#include "etj_iignore_repository.h"
#include <memory>

namespace ETJump
{
	class IgnoreService : public IIgnoreService
	{
	public:
		explicit IgnoreService(std::shared_ptr<IIgnoreRepository> ignoreRepository);
		~IgnoreService();

		void ignore(const std::string& callingPlayerGuid, const std::string& ignoredPlayerGuid, const std::string& ignoredPlayerName) override;
		void unignore(const std::string& callingPlayerGuid, int ignoreId) override;
		std::vector<Ignore> getIgnoredPlayers(const std::string& playerGuid) override;
		bool isIgnored(const std::string& callingPlayerGuid, const std::string& ignoredPlayerGuid) override;

	private:
		std::shared_ptr<IIgnoreRepository> _ignoreRepository;
	};
}
