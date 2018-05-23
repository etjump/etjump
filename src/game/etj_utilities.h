//
// Created by Jussi on 7.4.2015.
//

#ifndef ETJUMP_UTILITIES_H
#define ETJUMP_UTILITIES_H

#include <vector>
#include "etj_levels.h"

namespace Utilities {
	/**
	 * Returns the list of spectators spectating client
	 * @param clientNum The client
	 */
	std::vector<int> getSpectators(int clientNum);

	/**
	 * Disables all cheats for player. Resets saves
	 * @param clientNum the client
	 * TODO: Should create a Game-object that handles these
	 */
	void startRun(int clientNum);

	/**
	 * Enables all cheats for player.
	 * @param clientNum the player
	 */
	void stopRun(int clientNum);

	/**
	 * Gets a list of all maps on the server currently
	 */
	std::vector<std::string> getMaps();

	/**
	 * Sends an error message to server console
	 */
	void Error(const std::string& error);

	/**
	 * Sends a message to server console
	 */
	void Console(const std::string& message);

	/**
	* Log a message
	*/
	void Log(const std::string& message);

	/**
	* Log a message & appends a newline
	*/
	void Logln(const std::string& message);

	/**
	* Writes string to specified file
	*/
	void WriteFile(const std::string& filepath, const std::string& content);

	/**
	* Reads contents of specified file and returns std moved string
	*/
	std::string ReadFile(const std::string& filepath);

	/**
	 * Returns true if there is atleast one player on team
	 */
	bool anyonePlaying();

	/**
	 * Returns the q3 engine path for a file
	 */
	std::string getPath(const std::string& name);

	/**
	 * Converts timestamp to date
	 */
	std::string timestampToString(int timestamp, const char *format = "%d/%m/%y %H:%M:%S", const char *start = "never");

	/**
	 * Prints a message to entity's console
	 * if ent == nullptr, prints to server console
	 */
	void toConsole(gentity_t *ent, std::string message);

	void RemovePlayerWeapons(int clientNum, const std::vector<int>& weapons);
};


#endif //ETJUMP_UTILITIES_H
