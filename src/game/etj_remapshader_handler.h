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

#include "g_local.h"

namespace ETJump {

// 128 in VET for 32 shaders, upped to 2048 for our extended 512 shaders
inline constexpr int32_t MAX_SHADER_REMAPS = 2048;

class RemapShaderHandler {
public:
  void addRemap(std::string_view oldShader, std::string_view newShader);
  void updateShaderState();

  // so clients get the state of extended shaders on init
  void sendCurrentShaderStateExt();

private:
  struct ShaderRemap {
    std::string oldShader;
    std::string newShader;
    float timeOffset{};
  };

  std::array<ShaderRemap, MAX_SHADER_REMAPS> shaderRemaps{};
  std::array<std::string, EXT_SHADER_SET_COUNT> extShaderStates{};
  int32_t remapCount{};

  std::string buildShaderStateConfig(int32_t index);
  void sendExtShaderState(int32_t index);
};
} // namespace ETJump
