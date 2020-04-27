/*
 * MIT License
 * 
 * Copyright (c) 2020 ETJump team <haapanen.jussi@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "etj_deathrun_system.h"
#include <algorithm>


ETJump::DeathrunSystem::DeathrunSystem(): _defaultLocation(PrintLocation::Left)
{
}

ETJump::DeathrunSystem::~DeathrunSystem()
{
}

void ETJump::DeathrunSystem::addStartMessage(const std::string& startMessage)
{
	_startMessage = startMessage;
}

void ETJump::DeathrunSystem::addEndMessage(const std::string& endMessage)
{
	_endMessage = endMessage;
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
	_runStatuses[clientNum].active = false;
	
	// cannot be a range for loop as vector<bool> is special
	for (int i = 0, len = _runStatuses[clientNum].checkpointStatuses.size(); i < len; ++i)
	{
		_runStatuses[clientNum].checkpointStatuses[i] = false;
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

std::string ETJump::DeathrunSystem::getEndMessage() const
{
	return _endMessage;
}

std::string ETJump::DeathrunSystem::getMessageFormat(PrintLocation location)
{
	switch (location)
	{
	case ETJump::DeathrunSystem::PrintLocation::Chat:
		return "chat \"%s\"";
	case ETJump::DeathrunSystem::PrintLocation::Center:
		return "cp \"%s\n\"";
	case ETJump::DeathrunSystem::PrintLocation::Left:
		return "cpm \"%s\n\"";
	case ETJump::DeathrunSystem::PrintLocation::Console:
	default:
		return "print \"%s\n\"";
	}
}
