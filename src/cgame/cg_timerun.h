//
// Created by Jussi on 14.5.2015.
//

#ifndef ETJUMP_TIMERUN_H
#define ETJUMP_TIMERUN_H

#include <map>
#include <string>
#include "etj_drawable.hpp"

class Timerun : Drawable {
public:
	static const int NO_PREVIOUS_RECORD = -1;
	/**
	 * @param clientNum The user's client number
	 */
	Timerun(int clientNum);
	/**
	 * Draws the timer
	 */
	void draw();

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
	int _fastestTime;

	/**
	 * Own clientNum
	 */
	int _clientNum;

	/**
	 * Running player's clientNum. Can be either _clientNum or someone elses cnum
	 */
	int _runningPlayerClientNum;

	/**
	 * Fastest times for each player on each run
	 */
	std::map<int, std::map<std::string, int> > _fastestTimes;

	/**
	 * Whether we're currently on a timerun or not. Applies
	 * to both spectator & actual running
	 */
	bool _running;

	/**
	 * The name of the current timerun we're doing
	 */
	std::string _currentTimerun;

	/**
	 * The timestamp when user started the current run
	 */
	int _startTime;

	/**
	 * The time it took user to complete previous run
	 */
	int _completionTime;
};


#endif //ETJUMP_TIMERUN_H
