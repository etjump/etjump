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

#pragma once

#if defined(CGAMEDLL)
  #include "cg_local.h"
#elif defined(UIDLL)
  #include "../ui/ui_local.h"
#endif

#include "../game/etj_string_utilities.h"
#include "../game/etj_shared.h"

namespace ETJump {
class CvarValue {
public:
  struct Size {
    float x;
    float y;
  };

  struct Scale {
    float x;
    float y;
  };
};

class CvarValueParser {
public:
  template <typename T>
  static T parse(const vmCvar_t &cvar, opt<float> min = opt<float>(),
                 opt<float> max = opt<float>()) {
    const bool clamp = min.hasValue() && max.hasValue();

    // user set the string empty, return sane default values
    // we can't unfortunately get the real default value for a vmCvar_t easily
    if (std::strlen(cvar.string) == 0) {
      if (clamp) {
        return T{min.value(), min.value()};
      }

      return T{1.0f, 1.0f};
    }

    const auto scaleComponents = StringUtil::split(cvar.string, " ");

    // single component = uniform scaling
    if (scaleComponents.size() == 1) {
      float ufScale = Q_atof(scaleComponents[0].c_str());

      if (clamp) {
        ufScale = std::clamp(ufScale, min.value(), max.value());
      }

      return T{ufScale, ufScale};
    }

    float scaleX = Q_atof(scaleComponents[0].c_str());
    float scaleY = Q_atof(scaleComponents[1].c_str());

    if (clamp) {
      scaleX = std::clamp(scaleX, min.value(), max.value());
      scaleY = std::clamp(scaleY, min.value(), max.value());
    }

    return T{scaleX, scaleY};
  }
};
} // namespace ETJump
