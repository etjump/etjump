/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
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

/*
        Any player related events, excluding spectated players or other players.
        Currently added:
        *	respawn ( revived: "1" or "0")
        *	timerun:start ( clientNum, runname, starttime, recordtime )
        *	timerun:stop ( clientNum, runname, completionTime )
        *	timerun:record ( clientNum, runname, completionTime )
        *	timerun:completion ( clientNum, runname, completionTime )
        *	load
        add more as needed
*/
#pragma once

#ifdef min
  #undef min
#endif
#ifdef max
  #undef max
#endif

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <memory>

namespace ETJump {
class PlayerEventsHandler {
  std::map<std::string,
           std::vector<std::function<void(const std::vector<std::string> &)>>>
      _callbacks;

public:
  PlayerEventsHandler();
  ~PlayerEventsHandler();
  bool check(const std::string &event,
             const std::vector<std::string> &arguments);
  bool subscribe(
      const std::string &event,
      const std::function<void(const std::vector<std::string> &)> &callback);
  bool unsubscribe(const std::string &event);
};
} // namespace ETJump

#pragma once
