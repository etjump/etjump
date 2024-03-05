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

#include <string>

#include "etj_demo_compatibility.h"
#include "cg_local.h"
#include "../game/etj_string_utilities.h"

namespace ETJump {
DemoCompatibility::DemoCompatibility() {
  if (cg.demoPlayback) {
    parseDemoVersion();
  }
}

void DemoCompatibility::parseDemoVersion() {
  const char *serverInfoCS = CG_ConfigString(CS_SERVERINFO);
  std::string versionStr = Info_ValueForKey(serverInfoCS, "mod_version");

  // mod_version is present from '2.0.3 Alpha' onwards
  // if not present, look for sv_referencedPakNames as a fallback
  if (!versionStr.empty()) {
    fillVersionInfo(&demoVersion, versionStr, ".");
  } else {
    const char *sysInfoCS = CG_ConfigString(CS_SYSTEMINFO);
    const char *pakNames = Info_ValueForKey(sysInfoCS, "sv_referencedPakNames");

    // sanity check, shouldn't happen
    if (pakNames == nullptr) {
      return;
    }

    const char *pak = std::strchr(pakNames, '/'); // etjump/...

    // should not happen
    if (pak == nullptr) {
      return;
    }

    versionStr = pak;
    size_t idx = std::string::npos;

    for (size_t i = 0; i < versionStr.length(); i++) {
      if (Q_isnumeric(versionStr[i])) {
        idx = i;
        break;
      }
    }

    // parsing failed, bail
    if (idx == std::string::npos) {
      return;
    }

    versionStr.erase(0, idx);

    // pre 2.4.0 used '_' in pk3 name as delimiter
    fillVersionInfo(&demoVersion, versionStr, "_");
  }
}

void DemoCompatibility::fillVersionInfo(Version *version,
                                        const std::string &versionStr,
                                        const std::string &delimiter) {
  const auto splits = StringUtil::split(versionStr, delimiter);

  // bail if we have some weird version without at least 3 digits
  if (splits.size() < 3) {
    CG_Printf("^3WARNING: could not parse version compatibility info!\n");
    return;
  }

  version->major = std::stoi(splits[0].substr(0, 1));
  version->minor = std::stoi(splits[1].substr(0, 1));
  version->patch = std::stoi(splits[2].substr(0, 1));
}

bool DemoCompatibility::isCompatible(Version minimum) const {
  if (demoVersion.major != minimum.major) {
    return demoVersion.major > minimum.major;
  }

  if (demoVersion.minor != minimum.minor) {
    return demoVersion.minor > minimum.minor;
  }

  return demoVersion.patch >= minimum.patch;
}
} // namespace ETJump
