/*
 * MIT License
 * 
 * Copyright (c) 2022 ETJump team <zero@etjump.com>
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

#pragma once

#include <map>
#include <string>
#include "etj_drawable.h"

class Timerun {
public:
	static const int NO_PREVIOUS_RECORD = -1;
	/**
	 * @param clientNum The user's client number
	 */
	Timerun(int clientNum);

	/**
	 * Starts a time run for self. This is only called
	 * when player is actually running and not spectating
	 * someone running
	 * @param runName the run the player just started
	 */
	void startTimerun(const std::string& runName, int startTime, int previousRecord);

	/**
	 * @param clientNum Client's number who started the run
	 * @param runName Run that the client started
	 */
	void startSpectatorTimerun(int clientNum, const std::string& runName, int startTime, int previousRecord);

	/**
	 * @param completionTime How long it took to complete the run in ms
	 */
	void stopTimerun(int completionTime);

	/**
	 * Interrups the current run. Stops the timer and resets everything.
	 */
	void interrupt();

	/**
	 * @param clientNum The user's clientNum who completed the run
	 * @param completionTime The time it took to complete the run
	 * @param runName the name of the run
	 */
	void stopSpectatorTimerun(int clientNum, int completionTime, const std::string& runName);

	/**
	 * Called whenever client completes a run but it's not his fastest time
	 * @param clientNum Client's num
	 * @param runName Run name
	 * @param completionTime How long the run took to complete
	 */
	void completion(int clientNum, std::string runName, int completionTime);

	/**
	* Called whenever client beats his record in a run
	* @param clientNum Client's num
	* @param runName Run name
	* @param completionTime How long the run took to complete
	*/
	void record(int clientNum, std::string runName, int completionTime);
private:
	/**
	 * Get's _runningPlayerClients's fastest time on the _currentTimerun
	 */
	void getOwnFastestTime();

	/**
	 * Fastest time by _runningPlayerClientNum in _currentTimerun
	 */
	int _fastestTime{ -1 };

	/**
	 * Own clientNum
	 */
	int _clientNum;

	/**
	 * Running player's clientNum. Can be either _clientNum or someone elses cnum
	 */
	int _runningPlayerClientNum{ 0 };

	/**
	 * Fastest times for each player on each run
	 */
	std::map<int, std::map<std::string, int> > _fastestTimes;

	/**
	 * Whether we're currently on a timerun or not. Applies
	 * to both spectator & actual running
	 */
	bool _running{ false };

	/**
	 * The name of the current timerun we're doing
	 */
	std::string _currentTimerun{ "" };

	/**
	 * The timestamp when user started the current run
	 */
	int _startTime{ 0 };

	/**
	 * The time it took user to complete previous run
	 */
	int _completionTime{ 0 };

	struct Time
	{
		enum class Duration
		{
			Millisecond = 1,
			Second = 1000,
			Minute = 60 * 1000
		};
		int minutes;
		int seconds;
		int ms;
		int timestamp;
	};

	Time createTimeFromTimestamp(int timestamp);
	std::string createCompletionMessage(clientInfo_t& player, std::string& runName, int completionTime, int previousTime);
	std::string createTimeString(Time &time);
	void printMessage(std::string &message, int shaderIcon);
};
