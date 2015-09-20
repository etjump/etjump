//
// Created by Jussi on 7.4.2015.
//

#ifndef ETJUMP_UTILITIES_H
#define ETJUMP_UTILITIES_H

#include <vector>
#include "levels.hpp"

class Utilities {
public:
	/**
	 * Returns the list of spectators spectating client
	 * @param clientNum The client
	 */
	static std::vector<int> getSpectators(int clientNum);

	/**
	 * Disables all cheats for player. Resets saves
	 * @param clientNum the client
	 * TODO: Should create a Game-object that handles these
	 */
	static void startRun(int clientNum);

	/**
	 * Enables all cheats for player.
	 * @param clientNum the player
	 */
	static void stopRun(int clientNum);

	/**
	 * Gets a list of all maps on the server currently
	 */
	static std::vector<std::string> getMaps();

	/**
	 * Sends an error message to server console
	 */
	static void Error(const std::string& error);

	/**
	 * Sends a message to server console
	 */
	static void Console(const std::string& message);

	/**
	* Log a message
	*/
	static void Log(const std::string& message);

	/**
	 * Returns true if there is atleast one player on team
	 */
	static bool anyonePlaying();

	/**
	 * Returns the q3 engine path for a file
	 */
	static std::string getPath(const std::string& name);

	/**
	 * Converts timestamp to date
	 */
	static std::string timestampToString(int timestamp, const char *format = "%d/%m/%y %H:%M:%S", const char *start = "never");

	/**
	 * Prints a message to entity's console
	 * if ent == nullptr, prints to server console
	 */
	static void toConsole(gentity_t *ent, std::string message);
private:
};


#endif //ETJUMP_UTILITIES_H
