/*
 * MIT License
 *
 * Copyright (c) 2026 ETJump team <zero@etjump.com>
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

#pragma once
#include <string>
#include <functional>

#include "cg_local.h"

namespace ETJump {
typedef std::initializer_list<const char *> ShaderStage;
typedef std::initializer_list<ShaderStage> ShaderStages;
std::string composeShader(const char *name, ShaderStage general,
                          ShaderStages stages);
std::string composeShader(const char *name, ShaderStages stages);

template <typename T, std::size_t N>
constexpr std::size_t nelem(T (&)[N]) {
  return N;
}

int setTimeout(std::function<void()> fun, int delay);

bool clearTimeout(int handle);

int setInterval(std::function<void()> fun, int delay);

bool clearInterval(int handle);

int setImmediate(std::function<void()> fun);

bool clearImmediate(int handle);

void executeTimeout(int handle);

// checks if a cfg file with given name exists, omit .cfg extension
bool configFileExists(const std::string &filename);

// executes a cfg file with given name, omit .cfg extension
void execFile(const std::string &filename, ExecFileType type);

// returns true if clientNum is a valid number for client
bool isValidClientNum(int clientNum);

// returns true if client is currently playing (team axis/allies)
bool isPlaying(int clientNum);

// returns the server snapshot rate (sv_fps)
int getSvFps();

// checks if we should have collision with the target player
bool playerIsSolid(int self, int other);

// checks if target client is noclipping
bool playerIsNoclipping(int clientNum);

bool skipPortalDraw(int selfNum, int otherNum);

void registerGameShader(int32_t index, const char *shader);

void centerCursor();

void addLoopingSound(const vec3_t origin, const vec3_t velocity,
                     sfxHandle_t sfx, int volume, int soundTime);

void addRealLoopingSound(const vec3_t origin, const vec3_t velocity,
                         sfxHandle_t sfx, int range, int volume, int soundTime);

// true if 'entityNum' is a player with hideme enabled
bool hideMeCheck(int32_t entityNum);

// returns correct trace contents depending on 'etj_extraTrace' value
int32_t getExtraTraceContents(int32_t value);

int32_t weapnumForClient();

// true if scoreboard is visible
bool showingScores();

void onPlayerRespawn(bool revived);

playerState_t *getValidPlayerState();

void resetTransitionEffects();

void resetCustomvoteInfo();
} // namespace ETJump
