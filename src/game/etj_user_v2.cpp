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

#ifdef NEW_AUTH

  #include "etj_user_v2.h"
  #include "etj_string_utilities.h"
  #include "etj_time_utilities.h"
  #include "utilities.hpp"

namespace ETJump {
void UserV2::formatGreeting(const gentity_t *ent, std::string &greeting) const {
  StringUtil::replaceAll(greeting, "[n]", ent->client->pers.netname);

  const std::string lastSeenStr =
      lastSeen > 0 ? Time::fromInt(lastSeen).toDateTimeString() : "never";
  StringUtil::replaceAll(greeting, "[t]", lastSeenStr);

  time_t t = 0;
  t = std::time(&t);
  StringUtil::replaceAll(
      greeting, "[d]",
      TimeStampDifferenceToString(static_cast<int32_t>(t - lastSeen)));
}
} // namespace ETJump

#endif
