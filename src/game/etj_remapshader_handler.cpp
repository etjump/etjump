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

#include "etj_remapshader_handler.h"
#include "etj_string_utilities.h"
#include "etj_printer.h"

namespace ETJump {
void RemapShaderHandler::addRemap(const std::string_view oldShader,
                                  const std::string_view newShader) {
  const float timeOffset = static_cast<float>(level.time) * 0.001f;

  for (int32_t i = 0; i < remapCount; i++) {
    // already registered, just update and exit
    if (StringUtil::iEqual(oldShader, shaderRemaps[i].oldShader)) {
      shaderRemaps[i].newShader = newShader;
      shaderRemaps[i].timeOffset = timeOffset;
      return;
    }
  }

  // this is a new shader remap, register it if we have space
  if (remapCount >= MAX_SHADER_REMAPS) {
    return;
  }

  shaderRemaps[remapCount].oldShader = oldShader;
  shaderRemaps[remapCount].newShader = newShader;
  shaderRemaps[remapCount].timeOffset = timeOffset;

  const auto truncate = [](std::string &str, size_t maxLen) {
    if (str.size() >= maxLen) {
      str.resize(maxLen - 1);
    }
  };

  // make sure these don't overflow MAX_QPATH
  truncate(shaderRemaps[remapCount].oldShader, MAX_QPATH);
  truncate(shaderRemaps[remapCount].newShader, MAX_QPATH);

  remapCount++;
}

void RemapShaderHandler::updateShaderState() {
  std::string csState;

  for (auto &state : extShaderStates) {
    state.clear();
  }

  for (int32_t i = 0; i < remapCount; i++) {
    if (i < MAX_CS_SHADERS) {
      csState += buildShaderStateConfig(i);
    } else {
      const int32_t index = (i - MAX_CS_SHADERS) / MAX_CS_SHADERS;
      extShaderStates[index] += buildShaderStateConfig(i);
    }
  }

  if (!csState.empty()) {
    trap_SetConfigstring(CS_SHADERSTATE, csState.c_str());
  }

  sendCurrentShaderStateExt();
}

void RemapShaderHandler::sendCurrentShaderStateExt() {
  for (int32_t i = 0; i < extShaderStates.size(); i++) {
    if (!extShaderStates[i].empty()) {
      sendExtShaderState(i);
    }
  }
}

std::string RemapShaderHandler::buildShaderStateConfig(const int32_t index) {
  const int32_t i1 = G_ShaderIndex(shaderRemaps[index].oldShader.c_str());
  const int32_t i2 = G_ShaderIndex(shaderRemaps[index].newShader.c_str());

  return stringFormat("%i=%i:%5.2f@", i1, i2, shaderRemaps[index].timeOffset);
}

void RemapShaderHandler::sendExtShaderState(const int32_t index) {
  Printer::commandAll(
      stringFormat("extShaderState %s", extShaderStates[index]));
}
} // namespace ETJump
