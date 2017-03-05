#include "etj_ignore_repository.h"

ETJump::IgnoreRepository::IgnoreRepository()
{
}

ETJump::IgnoreRepository::~IgnoreRepository()
{
}

void ETJump::IgnoreRepository::add(const std::string& forPlayerGuid, const std::string& targetPlayerGuid, const std::string& targetPlayerName)
{
}

void ETJump::IgnoreRepository::remove(const std::string& forPlayerGuid, int ignoreId)
{
}

std::vector<ETJump::Ignore> ETJump::IgnoreRepository::getIgnoredPlayers(const std::string& playerGuid)
{
}
