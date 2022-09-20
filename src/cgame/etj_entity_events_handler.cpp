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

#include "etj_entity_events_handler.h"
#include "../game/etj_string_utilities.h"

ETJump::EntityEventsHandler::EntityEventsHandler() { _callbacks.clear(); }

bool ETJump::EntityEventsHandler::check(const std::string &eventName,
                                        centity_t *cent) {
  auto lowercasedCommand = ETJump::StringUtil::toLowerCase(eventName);
  auto match = _callbacks.find(lowercasedCommand);
  if (match != end(_callbacks)) {
    for (auto callback : match->second) {
      callback(cent);
    }
    return true;
  }
  return false;
}

bool ETJump::EntityEventsHandler::check(int event, centity_t *cent) {
  auto eventName = ETJump::stringFormat("__event__%d", event);
  return check(eventName, cent);
}

bool ETJump::EntityEventsHandler::subscribe(
    const std::string &eventName,
    std::function<void(centity_t *cent)> callback) {
  auto lowercasedCommand = ETJump::StringUtil::toLowerCase(eventName);
  _callbacks[lowercasedCommand].push_back(callback);
  return true;
}

bool ETJump::EntityEventsHandler::subscribe(
    int event, std::function<void(centity_t *cent)> callback) {
  auto eventName = ETJump::stringFormat("__event__%d", event);
  return subscribe(eventName, callback);
}

bool ETJump::EntityEventsHandler::unsubcribe(const std::string &eventName) {
  auto lowercasedCommand = ETJump::StringUtil::toLowerCase(eventName);
  auto callback = _callbacks.find(lowercasedCommand);
  if (callback != end(_callbacks)) {
    return false;
  }

  _callbacks.erase(callback);
  return true;
}

bool ETJump::EntityEventsHandler::unsubcribe(int event) {
  auto eventName = ETJump::stringFormat("__event__%d", event);
  return unsubcribe(eventName);
}
