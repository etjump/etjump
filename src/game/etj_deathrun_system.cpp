#include "etj_deathrun_system.h"


ETJump::DeathrunSystem::DeathrunSystem()
{
}

ETJump::DeathrunSystem::~DeathrunSystem()
{
}

int ETJump::DeathrunSystem::createCheckpoint()
{
	for (auto & rs : _runStatuses)
	{
		rs.checkpointStatuses.push_back(false);
	}
	return _runStatuses[0].checkpointStatuses.size() - 1;
}

bool ETJump::DeathrunSystem::hitStart(int clientNum)
{
	if (isActive(clientNum))
	{
		return false;
	}

	setActive(clientNum);

	return true;
}

bool ETJump::DeathrunSystem::hitCheckpoint(int checkpointId, int clientNum)
{
	if (!isActive(clientNum))
	{
		return false;
	}

	if (alreadyReached(checkpointId, clientNum))
	{
		return false;
	}

	_runStatuses[clientNum].checkpointStatuses[checkpointId] = true;
	return true;
}

int ETJump::DeathrunSystem::hitEnd(int clientNum)
{
	auto score = getScore(clientNum);
	auto& runStatus = _runStatuses[clientNum];
	runStatus.active = false;
	for (auto & cpStatus : runStatus.checkpointStatuses)
	{
		cpStatus = false;
	}
	return score;
}

bool ETJump::DeathrunSystem::isActive(int clientNum)
{
	return _runStatuses[clientNum].active;
}

void ETJump::DeathrunSystem::setActive(int clientNum)
{
	_runStatuses[clientNum].active = true;
}

bool ETJump::DeathrunSystem::alreadyReached(int checkpointId, int clientNum)
{
	return _runStatuses[clientNum].checkpointStatuses[checkpointId];
}

int ETJump::DeathrunSystem::getScore(int clientNum)
{
	return std::count_if(begin(_runStatuses[clientNum].checkpointStatuses), end(_runStatuses[clientNum].checkpointStatuses), [](bool b)
	{
		return b;
	});
}
