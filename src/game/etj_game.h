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

#include <memory>

namespace ETJump {
class TimerunV2;
class RockTheVote;
class Tokens;
class ChatReplay;
class Motd;
class CustomMapVotes;
class MapStatistics;
class FireteamCountdown;
} // namespace ETJump

class Levels;
class Commands;
class Timerun;

struct Game {
  Game() = default;

  std::shared_ptr<Levels> levels;
  std::shared_ptr<Commands> commands;
  std::shared_ptr<ETJump::MapStatistics> mapStatistics;
  std::shared_ptr<ETJump::TimerunV2> timerunV2;
  std::shared_ptr<ETJump::RockTheVote> rtv;

  std::unique_ptr<ETJump::CustomMapVotes> customMapVotes;
  std::unique_ptr<ETJump::Motd> motd;
  std::unique_ptr<ETJump::Tokens> tokens;
  std::unique_ptr<ETJump::ChatReplay> chatReplay;
  std::unique_ptr<ETJump::FireteamCountdown> fireteamCountdown;
};
