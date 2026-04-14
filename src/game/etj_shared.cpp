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

#include "etj_shared.h"
#include "bg_public.h"
#include "etj_string_utilities.h"
#include "etj_worldspawn.h"

namespace ETJump {
int32_t activeSharedValue(
    int32_t shared,
    const std::vector<std::pair<std::string, int32_t>> &ignoredKeys,
    team_t team, bool timerunActive) {
  int32_t sharedValue = shared;

  const auto ignoredForTeam = [](const EnumBitset<WSModificationsFlags> &value,
                                 team_t currentTeam) {
    switch (currentTeam) {
      case TEAM_AXIS:
        return value & WSModificationsFlags::IGNORE_AXIS;
      case TEAM_ALLIES:
        return value & WSModificationsFlags::IGNORE_ALLIES;
      case TEAM_SPECTATOR:
        return value & WSModificationsFlags::IGNORE_SPEC;
      default:
        return false;
    }
  };

  const auto ignoredForTimerunState =
      [timerunActive](const EnumBitset<WSModificationsFlags> &value) {
        if (!(value & WSModificationsFlags::IGNORE_TIMERUN_ONLY) &&
            !(value & WSModificationsFlags::IGNORE_NO_TIMERUN)) {
          return true;
        }

        if (timerunActive &&
            (value & WSModificationsFlags::IGNORE_TIMERUN_ONLY)) {
          return true;
        }

        if (!timerunActive &&
            (value & WSModificationsFlags::IGNORE_NO_TIMERUN)) {
          return true;
        }

        return false;
      };

  // we only need to parse the keys that are relevant to either pmove
  // or cgame drawables here (area indicators, OB detector)
  for (const auto &[key, val] : ignoredKeys) {
    EnumBitset<WSModificationsFlags> value(val);

    if (StringUtils::iEqual(key, Worldspawn::Keys::NO_OVERBOUNCE)) {
      if (ignoredForTeam(value, team) && ignoredForTimerunState(value)) {
        sharedValue &= ~BG_LEVEL_NO_OVERBOUNCE;
      }
    } else if (StringUtils::iEqual(key, Worldspawn::Keys::NO_JUMP_DELAY)) {
      if (ignoredForTeam(value, team) && ignoredForTimerunState(value)) {
        sharedValue &= ~BG_LEVEL_NO_JUMPDELAY;
      }
    } else if (StringUtils::iEqual(key, Worldspawn::Keys::NO_SAVE)) {
      if (ignoredForTeam(value, team) && ignoredForTimerunState(value)) {
        sharedValue &= ~BG_LEVEL_NO_SAVE;
      }
    } else if (StringUtils::iEqual(key, Worldspawn::Keys::NO_PRONE)) {
      if (ignoredForTeam(value, team) && ignoredForTimerunState(value)) {
        sharedValue &= ~BG_LEVEL_NO_PRONE;
      }
    } else if (StringUtils::iEqual(key, Worldspawn::Keys::NO_WALLBUG)) {
      if (ignoredForTeam(value, team) && ignoredForTimerunState(value)) {
        sharedValue &= ~BG_LEVEL_NO_WALLBUG;
      }
    } else if (StringUtils::iEqual(key, Worldspawn::Keys::OVERBOUCNE_PLAYERS)) {
      if (ignoredForTeam(value, team) && ignoredForTimerunState(value)) {
        // clear both options since we want to ignore this completely
        sharedValue &= ~BG_LEVEL_BODY_OB_ALWAYS;
        sharedValue &= ~BG_LEVEL_BODY_OB_NEVER;
      }
    }
  }

  return sharedValue;
}
} // namespace ETJump
