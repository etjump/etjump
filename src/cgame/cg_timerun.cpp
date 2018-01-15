//
// Created by Jussi on 14.5.2015.
//

#include "cg_timerun.h"
#include <string>
#include <boost/format.hpp>
#include "../game/etj_string_utilities.h"

Timerun::Timerun(int clientNum)
{
	_clientNum = clientNum;
}

void Timerun::startTimerun(const std::string &runName, int startTime, int previousRecord)
{
	_running                = true;
	_startTime              = startTime;
	_currentTimerun         = runName;
	_runningPlayerClientNum = _clientNum;

	auto playerTimes = _fastestTimes.find(_clientNum);
	if (playerTimes == _fastestTimes.end())
	{
		_fastestTimes[_clientNum] = std::map<std::string, int>();
		playerTimes               = _fastestTimes.find(_clientNum);
	}

	playerTimes->second[runName] = previousRecord;
	_fastestTime                 = previousRecord;
}

void Timerun::startSpectatorTimerun(int clientNum, const std::string &runName, int startTime, int previousRecord)
{
	if (clientNum == _clientNum)
	{
		return;
	}

	_running                = true;
	_currentTimerun         = runName;
	_startTime              = startTime;
	_runningPlayerClientNum = clientNum;

	auto playerTimes = _fastestTimes.find(clientNum);
	if (playerTimes == _fastestTimes.end())
	{
		_fastestTimes[clientNum] = std::map<std::string, int>();
		playerTimes              = _fastestTimes.find(clientNum);
	}

	playerTimes->second[runName] = previousRecord;
	_fastestTime                 = previousRecord;
}

void Timerun::interrupt()
{
	_running                = false;
	_currentTimerun         = "";
	_startTime              = 0;
	_runningPlayerClientNum = 0;
	_fastestTime            = -1;
}

void Timerun::stopTimerun(int completionTime)
{
	_running        = false;
	_completionTime = completionTime;
}

void Timerun::record(int clientNum, std::string runName, int completionTime)
{

	int previousTime = NO_PREVIOUS_RECORD;

	auto playerTimes = _fastestTimes.find(clientNum);
	if (playerTimes != _fastestTimes.end())
	{
		auto previousTimeIter = playerTimes->second.find(runName);
		if (previousTimeIter != playerTimes->second.end())
		{
			previousTime = previousTimeIter->second;
		}
	}
	std::string message = createCompletionMessage(cgs.clientinfo[clientNum], runName, completionTime, previousTime);
	int shader = previousTime == NO_PREVIOUS_RECORD ?
		cgs.media.stopwatchIcon : cgs.media.stopwatchIconGreen;
	printMessage(message, shader);
}


void Timerun::completion(int clientNum, std::string runName, int completionTime)
{
	int  previousRecord = NO_PREVIOUS_RECORD;
	auto playerTimes    = _fastestTimes.find(clientNum);
	if (playerTimes != _fastestTimes.end())
	{
		auto previousTimeIter = playerTimes->second.find(runName);
		if (previousTimeIter != playerTimes->second.end())
		{
			previousRecord = previousTimeIter->second;
		}
	}

	// if we're the player / spectating the player, print the message
	if (clientNum == cg.snap->ps.clientNum)
	{
		std::string message = createCompletionMessage(cgs.clientinfo[clientNum], runName, completionTime, previousRecord);
		printMessage(message, cgs.media.stopwatchIconRed);
	}
}

void Timerun::stopSpectatorTimerun(int clientNum, int completionTime, const std::string& currentRun)
{
	_running        = false;
	_completionTime = completionTime;
}

Timerun::Time Timerun::createTimeFromTimestamp(int timestamp)
{
	int millis = timestamp;

	int hours = millis / static_cast<int>(Time::Duration::Hour);
	millis -= hours * static_cast<int>(Time::Duration::Hour);

	int minutes = millis / static_cast<int>(Time::Duration::Minute);
	millis -= minutes * static_cast<int>(Time::Duration::Minute);

	int seconds = millis / static_cast<int>(Time::Duration::Second);
	millis -= seconds * static_cast<int>(Time::Duration::Second);

	return{ hours, minutes, seconds, millis, timestamp };
}

std::string Timerun::createCompletionMessage(clientInfo_t& player, std::string& runName, int completionTime, int previousTime)
{
	Time now = createTimeFromTimestamp(completionTime);
	std::string who{ (player.clientNum == _clientNum) ? "You" : player.name };
	std::string timeFinished{ createTimeString(now) };
	std::string timeDifference{ "" };
	auto postfix = '!';

	if (previousTime != NO_PREVIOUS_RECORD)
	{
		Time diff = createTimeFromTimestamp(abs(previousTime - completionTime));
		std::string timeDir = (previousTime > completionTime) ?  "-^2" : "+^1";
		timeDifference = ETJump::stringFormat("^7(%s%s^7)", timeDir, createTimeString(diff));
		postfix = (previousTime > completionTime) ? '!' : '.';
	}

	std::string message = ETJump::stringFormat(
		"^7%s completed ^7%s ^7in %s%c %s", who, runName, timeFinished, postfix, timeDifference
	);

	return message;
}

std::string Timerun::createTimeString(Time &time)
{
	return ETJump::stringFormat("%02d:%02d.%03d", time.minutes, time.seconds, time.ms);
}

void Timerun::printMessage(std::string &message, int shaderIcon)
{
	CG_AddPMItem(PM_MESSAGE, message.c_str(), shaderIcon);
}
