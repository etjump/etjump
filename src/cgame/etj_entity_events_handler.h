/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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

#include "cg_local.h"

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
class EntityEventsHandler {
public:
  explicit EntityEventsHandler();

  // returns true if a match was found and function was called
  bool check(const std::string &eventName, centity_t *cent);
  bool check(int event, centity_t *cent);

  // registers a event handler that will be called if the event was
  // received from the server
  bool subscribe(const std::string &eventName,
                 const std::function<void(centity_t *cent)> &callback);
  bool subscribe(int event,
                 const std::function<void(centity_t *cent)> &callback);

  // unsubscribes the event handler
  // returns false if it does not exist
  bool unsubscribe(const std::string &eventName);
  bool unsubscribe(int event);

private:
  std::map<std::string, std::vector<std::function<void(centity_t *cent)>>>
      _callbacks;
};
} // namespace ETJump
