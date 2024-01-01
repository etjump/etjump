/*
 * MIT License
 *
 * Copyright (c) 2024 ETJump team <zero@etjump.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef ETJUMP_UTILITIES_H
#define ETJUMP_UTILITIES_H

#include <algorithm>
#include <iterator>
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
void Error(const std::string &error);

/**
 * Sends a message to server console
 */
void Console(const std::string &message);

/**
 * Log a message
 */
void Log(const std::string &message);

/**
 * Log a message & appends a newline
 */
void Logln(const std::string &message);

/**
 * Writes string to specified file
 */
void WriteFile(const std::string &filepath, const std::string &content);

/**
 * Reads contents of specified file and returns std moved string
 */
std::string ReadFile(const std::string &filepath);

/**
 * Returns true if there is atleast one player on team
 */
bool anyonePlaying();

/**
 * Returns the q3 engine path for a file
 */
std::string getPath(const std::string &name);

/**
 * Converts timestamp to date
 */
std::string timestampToString(int timestamp,
                              const char *format = "%d/%m/%y %H:%M:%S",
                              const char *start = "never");

void RemovePlayerWeapons(int clientNum);
} // namespace Utilities

namespace ETJump {
} // namespace ETJump

#endif // ETJUMP_UTILITIES_H
