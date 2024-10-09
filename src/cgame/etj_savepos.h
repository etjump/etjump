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

#pragma once

#include <unordered_map>
#include "../game/etj_savepos_shared.h"
#include "etj_timerun.h"

namespace ETJump {
class SavePos {
  enum class PosFlags {
    NoVelocity = 1,
    NoPitch = 2,
  };

  const std::string defaultName = "default";

  std::unordered_map<std::string, SavePosData> savePositions;

  // writes the savepos data to a file
  void writeSaveposFile(SavePosData &data);

  // adds new savepos to savePositions map, or overwrites an existing one
  void storePosition(const SavePosData &data);

  std::shared_ptr<Timerun> timerun = nullptr;

  static PlayerStance getStance(const playerState_t *ps);

  std::string errors;

public:
  explicit SavePos(const std::shared_ptr<Timerun> &p);
  ~SavePos() = default;

  // parses all existing savepos files and adds them to the savePositions map
  // this is called on constructor to store existing positions to memory,
  // and can be called manually to reload positions using 'readsavepos' command
  // if manual is true, prints out number of positions even if no positions
  // are loaded, to give some output to 'readsavepos' command
  void parseExistingPositions(bool manual);

  // processes the players current state and fills out Position struct
  void createSaveposData(const std::string &file, int flags);

  // parses existing savepos file
  void parseSavepos(const std::string &file);

  // returns the data for given savepos
  const SavePosData &getSaveposData(const std::string &name);

  // returns default savepos name
  std::string getDefaultSaveposName() const;

  // returns all valid savepos names
  std::vector<std::string> getSaveposNames();

  // check if a given savepos exists (used with loadpos)
  bool saveposExists(const std::string &name);
};
} // namespace ETJump
