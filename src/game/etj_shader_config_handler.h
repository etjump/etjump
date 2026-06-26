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

#include "q_shared.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ETJump {
// 128 in VET for 32 shaders, completely arbitrary
inline constexpr int32_t MAX_SHADER_REMAPS = MAX_SHADER_INDEX * 4;

class ShaderConfigHandler {
public:
  int16_t getShaderIndex(const char *name, bool create);
  void addShaderRemap(const char *oldShader, const char *newShader);
  void updateShaderState();

private:
  struct ShaderRemap {
    std::string originalShader;
    std::string currentShader;
    float timeOffset{};
    int16_t i1; // 'originalShader' index
    int16_t i2; // 'currentShader index
  };

  std::unordered_map<std::string, int16_t> gameShaderCache;
  std::vector<ShaderRemap> shaderRemaps;
  std::unordered_set<int16_t> remapQueue;

  std::string buildShaderStateString(int32_t index);
};
} // namespace ETJump
