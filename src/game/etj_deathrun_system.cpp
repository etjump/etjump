#include "etj_deathrun_system.h"


ETJump::DeathrunSystem::DeathrunSystem()
{
}

ETJump::DeathrunSystem::~DeathrunSystem()
{
}

void ETJump::DeathrunSystem::addStartMessage(const std::string& startMessage)
{
	_startMessage = startMessage;
}

void ETJump::DeathrunSystem::addDefaultCheckpointMessage(const std::string& defaultMessage)
{
	_defaultMessage = defaultMessage;
}

void ETJump::DeathrunSystem::addDefaultSoundPath(const std::string& defaultSoundPath)
{
	_defaultSoundPath = defaultSoundPath;
}

void ETJump::DeathrunSystem::addStartAndCheckpointMessageLocation(PrintLocation location)
{
	_defaultLocation = location;
}

int ETJump::DeathrunSystem::createCheckpoint(PrintLocation location, const std::string& message, const std::string& soundPath)
{
	for (auto & rs : _runStatuses)
	{
		rs.checkpointStatuses.push_back(false);
	}
	_checkpointData.push_back({ location, message, soundPath });
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

ETJump::DeathrunSystem::PrintLocation ETJump::DeathrunSystem::getPrintLocation() const
{
	return _defaultLocation;
}

ETJump::DeathrunSystem::PrintLocation ETJump::DeathrunSystem::getPrintLocation(int checkpointId)
{
	auto location = _defaultLocation;
	if (_checkpointData[checkpointId].location != PrintLocation::Unspecified)
	{
		location = _checkpointData[checkpointId].location;
	}
	return location;
}

std::string ETJump::DeathrunSystem::getStartMessage() const
{
	return _startMessage;
}

std::string ETJump::DeathrunSystem::getCheckpointMessage(int checkpointId) const
{
	if (_checkpointData[checkpointId].message.length() == 0)
	{
		return _defaultMessage;
	}
	return _checkpointData[checkpointId].message;
}

std::string ETJump::DeathrunSystem::getSoundPath(int checkpointId) const
{
	if (_checkpointData[checkpointId].soundPath.length() == 0)
	{
		return _defaultSoundPath;
	}
	return _checkpointData[checkpointId].soundPath;
}
