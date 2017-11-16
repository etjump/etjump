//
// Created by Jussi on 14.5.2015.
//

#include "cg_timerun.h"
#include "cg_local.h"
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <string>
#include <boost/format.hpp>

Timerun::Timerun(int clientNum)
	: Drawable(), _fastestTime(-1), _clientNum(clientNum), _runningPlayerClientNum(0), _running(false), _currentTimerun(""), _startTime(0), _completionTime(0)
{
}

void Timerun::draw()
{
	/*if (player_drawRunTimer.integer == 0 || !cg.hasTimerun)
	{
	    return;
	}

	auto startTime = _startTime;
	auto millis    = 0;

	if (_running)
	{
	    millis = cg.time - startTime;
	}
	else
	{
	    millis = _completionTime;
	}

	vec4_t *color = &colorWhite;

	if (_fastestTime > 0)
	{
	    if (millis > _fastestTime)
	    {
	        color = &colorRed;
	    }
	}

	auto minutes = millis / 60000;
	millis -= minutes * 60000;
	auto seconds = millis / 1000;
	millis -= seconds * 1000;

	auto text = (boost::format("%02d:%02d:%03d")
	             % minutes
	             % seconds
	             % millis).str();

	auto textWidth = CG_Text_Width_Ext(text.c_str(), 0.3, 0, &cgs.media.limboFont1) / 2;
	auto x         = player_runTimerX.integer;
	auto y         = player_runTimerY.integer;

	CG_Text_Paint_Ext(x - textWidth, y, 0.3, 0.3, *color, text.c_str(), 0, 0, 0, &cgs.media.limboFont1);*/
}

void Timerun::startTimerun(const std::string &runName, int startTime, int previousRecord)
{
	_running = true;
	_startTime = startTime;
	_currentTimerun = runName;
	_runningPlayerClientNum = _clientNum;

	auto playerTimes = _fastestTimes.find(_clientNum);
	if (playerTimes == _fastestTimes.end())
	{
		_fastestTimes[_clientNum] = std::map<std::string, int>();
		playerTimes = _fastestTimes.find(_clientNum);
	}

	playerTimes->second[runName] = previousRecord;
	_fastestTime = previousRecord;
}

void Timerun::startSpectatorTimerun(int clientNum, const std::string &runName, int startTime, int previousRecord)
{
	if (clientNum == _clientNum)
	{
		return;
	}

	_running = true;
	_currentTimerun = runName;
	_startTime = startTime;
	_runningPlayerClientNum = clientNum;

	auto playerTimes = _fastestTimes.find(clientNum);
	if (playerTimes == _fastestTimes.end())
	{
		_fastestTimes[clientNum] = std::map<std::string, int>();
		playerTimes = _fastestTimes.find(clientNum);
	}

	playerTimes->second[runName] = previousRecord;
	_fastestTime = previousRecord;
}

void Timerun::interrupt()
{
	_running = false;
	_currentTimerun = "";
	_startTime = 0;
	_runningPlayerClientNum = 0;
	_fastestTime = -1;
}

void Timerun::stopTimerun(int completionTime)
{
	_running = false;
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

	auto millis  = completionTime;
	auto minutes = millis / 60000;
	millis -= minutes * 60000;
	auto seconds = millis / 1000;
	millis -= seconds * 1000;

	auto diffMillis  = abs(completionTime - previousTime);
	auto diffMinutes = diffMillis / 60000;
	diffMillis -= minutes * 60000;
	auto diffSeconds = diffMillis / 1000;
	diffMillis -= diffSeconds * 1000;

	if (previousTime != NO_PREVIOUS_RECORD)
	{
		CG_AddPMItem(PM_MESSAGE, (boost::format("^7%s ^7completed ^7%s ^7in %02d:%02d:%03d! ^7(-^2%02d:%02d:%03d^7)")
			                      % cgs.clientinfo[clientNum].name
			                      % runName
			                      % minutes
			                      % seconds
			                      % millis
			                      % diffMinutes
			                      % diffSeconds
			                      % diffMillis).str().c_str(), cgs.media.voiceChatShader);
	}
	else
	{
		CG_AddPMItem(PM_MESSAGE, (boost::format("^7%s ^7completed %s in %02d:%02d:%03d!")
			                      % cgs.clientinfo[clientNum].name
			                      % runName
			                      % minutes
			                      % seconds
			                      % millis).str().c_str(), cgs.media.voiceChatShader);
	}
}


void Timerun::completion(int clientNum, std::string runName, int completionTime)
{
	int  previousRecord = NO_PREVIOUS_RECORD;
	auto playerTimes = _fastestTimes.find(clientNum);

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
		auto millis  = completionTime;
		auto minutes = millis / 60000;
		millis -= minutes * 60000;
		auto seconds = millis / 1000;
		millis -= seconds * 1000;

		if (previousRecord != NO_PREVIOUS_RECORD)
		{
			auto diffMillis  = abs(completionTime - previousRecord);
			auto diffMinutes = diffMillis / 60000;
			diffMillis -= minutes * 60000;
			auto diffSeconds = diffMillis / 1000;
			diffMillis -= diffSeconds * 1000;

			if (clientNum == _clientNum)
			{
				CG_AddPMItem(PM_MESSAGE, (boost::format("^7You completed ^7%s ^7in %02d:%02d:%03d (+^1%02d:%02d:%03d^7).")
					                      % runName
					                      % minutes
					                      % seconds
					                      % millis
					                      % diffMinutes
					                      % diffSeconds
					                      % diffMillis).str().c_str(), cgs.media.voiceChatShader);
			}
			else
			{
				CG_AddPMItem(PM_MESSAGE, (boost::format("^7%s ^7completed ^7%s ^7in %02d:%02d:%03d (+^1%02d:%02d:%03d^7).")
					                      % cgs.clientinfo[clientNum].name
					                      % runName
					                      % minutes
					                      % seconds
					                      % millis
					                      % diffMinutes
					                      % diffSeconds
					                      % diffMillis).str().c_str(), cgs.media.voiceChatShader);
			}
		}
		else
		{
			if (clientNum == _clientNum)
			{
				CG_AddPMItem(PM_MESSAGE, (boost::format("^7You completed ^7%s ^7in %02d:%02d:%03d.")
					                      % runName
					                      % minutes
					                      % seconds
					                      % millis).str().c_str(), cgs.media.voiceChatShader);
			}
			else
			{
				CG_AddPMItem(PM_MESSAGE, (boost::format("^7%s ^7completed ^7%s ^7in %02d:%02d:%03d.")
					                      % cgs.clientinfo[clientNum].name
					                      % runName
					                      % minutes
					                      % seconds
					                      % millis).str().c_str(), cgs.media.voiceChatShader);
			}
		}
	}
}

void Timerun::stopSpectatorTimerun(int clientNum, int completionTime, const std::string& currentRun)
{
	_running = false;
	_completionTime = completionTime;
}
