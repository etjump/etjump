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

#include "cg_local.h"

namespace ETJump {
// 2.3.0 (+ RC4) ET_VELOCITY_PUSH_TRIGGER eType number
inline constexpr int32_t COMPAT_ET_VELOCITY_PUSH_TRIGGER_NUM = 9;
// old array index of removed 'weapon_medic_heal' item
inline constexpr int32_t COMPAT_WEAPON_MEDIC_HEAL_INDEX = 56;

class DemoCompatibility {
  struct Version {
    int major;
    int minor;
    int patch;
  };

  void parseDemoVersion();
  void fillVersionInfo(Version &version, const std::string &versionStr,
                       const std::string &delimiter);

  void setupCompatibilityFlags();

  // returns true if the demo version is newer or same as 'minimum'
  [[nodiscard]] bool isCompatible(const Version &minimum) const;

  // returns true if 'version' is the exact same as demo version
  [[nodiscard]] bool isExactVersion(const Version &version) const;

  Version demoVersion{};

public:
  struct CompatibilityFlags {
    bool serverSideCoronas = false;
    bool svFpsUnavailable = false;
    bool svFpsInSysteminfo = false;
    bool adjustEntityTypes = false;
    bool noSavePosTimerunInfo = false;
    bool adjustEvTokens = false;
    bool adjustEvGeneralClientSoundVolume = false;
    bool adjustEvVelocityPushTrigger = false;
    bool adjustEvFakebrushAndClientTeleporter = false;
    bool serverSideDlights = false;
    bool setAttack2FiringFlag = false;
    bool adjustItemlistIndex = false;
    bool predictedJumpSpeeds = false;
    bool noSpecCountInVoteCs = false;
  };

  // everything in here will be set to false unless we're on demo playback
  CompatibilityFlags flags{};

  // stores the strings to print for compatibility info
  std::vector<std::string> compatibilityStrings;

  // performs event number adjustments for events that are added freestanding
  // by setting event num to ET_EVENTS + event due to additional entity types
  [[nodiscard]] int adjustedEventNum(int event) const;

  void printDemoInformation() const;

  DemoCompatibility();
  ~DemoCompatibility() = default;
};
} // namespace ETJump
