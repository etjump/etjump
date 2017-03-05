#include "etj_ignore_service.h"

ETJump::IgnoreService::IgnoreService(std::shared_ptr<IIgnoreRepository> ignoreRepository)
	:_ignoreRepository(ignoreRepository)
{
}

ETJump::IgnoreService::~IgnoreService()
{
}

void ETJump::IgnoreService::ignore(const std::string& callingPlayerGuid, const std::string& ignoredPlayerGuid, const std::string& ignoredPlayerName)
{
	_ignoreRepository->add(callingPlayerGuid, ignoredPlayerGuid, ignoredPlayerName);
}

void ETJump::IgnoreService::unignore(const std::string& callingPlayerGuid, int ignoreId)
{
	_ignoreRepository->remove(callingPlayerGuid, ignoreId);
}

std::vector<ETJump::Ignore> ETJump::IgnoreService::getIgnoredPlayers(const std::string& playerGuid)
{
	return _ignoreRepository->getIgnoredPlayers(playerGuid);
}

bool ETJump::IgnoreService::isIgnored(const std::string& callingPlayerGuid, const std::string& ignoredPlayerGuid)
{
	auto ignoredPlayers = _ignoreRepository->getIgnoredPlayers(callingPlayerGuid);

	for (const auto & p : ignoredPlayers)
	{
		if (p.targetPlayerGuid == ignoredPlayerGuid)
		{
			return true;
		}
	}
	return false;
}
