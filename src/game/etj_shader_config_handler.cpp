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

#include "etj_shader_config_handler.h"
#include "etj_string_utilities.h"
#include "g_local.h"

// ShaderConfigHandler - manages shader & shaderstate configstrings

namespace ETJump {
int16_t ShaderConfigHandler::getShaderIndex(const char *name,
                                            const bool create) {
  if (!name || name[0] == '\0') {
    return 0;
  }

  const auto it = gameShaderCache.find(name);

  // already registered
  if (it != gameShaderCache.cend()) {
    return it->second;
  }

  if (!create) {
    return 0;
  }

  if (gameShaderCache.size() == MAX_SHADER_INDEX) {
    G_Error(
        "%s: configstring overflow - MAX_SHADER_INDEX reached (%i), decrease "
        "the number of unique shaders referenced by mapscript and entities\n",
        __func__, MAX_SHADER_INDEX);
  }

  const auto index = static_cast<int16_t>(gameShaderCache.size() + 1);
  gameShaderCache[name] = index;

  const int32_t slot = (index - 1) / MAX_SHADERS_PER_INDEX;

  char buf[MAX_STRING_CHARS];
  trap_GetConfigstring(CS_SHADERS + slot, buf, sizeof(buf));
  Q_strcat(buf, sizeof(buf), va("%s ", name));
  trap_SetConfigstring(CS_SHADERS + slot, buf);

  return index;
}

void ShaderConfigHandler::addShaderRemap(const char *oldShader,
                                         const char *newShader) {
  const float timeOffset = static_cast<float>(level.time) * 0.001f;

  // check for an existing entry first, and just update it if found
  for (int32_t i = 0; i < shaderRemaps.size(); i++) {
    if (StringUtils::iEqual(shaderRemaps[i].originalShader, oldShader)) {
      shaderRemaps[i].currentShader = newShader;
      shaderRemaps[i].timeOffset = timeOffset;
      shaderRemaps[i].i2 = getShaderIndex(newShader, true);

      remapQueue.insert(static_cast<int16_t>(i / MAX_SHADERSTATES_PER_INDEX));
      return;
    }
  }

  // bail if we're out of space for remap combinations
  // FIXME: remove? I think this is not possible to hit anyway
  if (shaderRemaps.size() == MAX_SHADER_REMAPS) {
    return;
  }

  // new remap, register
  ShaderRemap sr{};
  sr.originalShader = oldShader;
  sr.currentShader = newShader;
  sr.timeOffset = timeOffset;

  const auto truncate = [](std::string &str, size_t maxLen) {
    if (str.size() >= maxLen) {
      str.resize(maxLen - 1);
    }
  };

  // make sure these don't overflow MAX_QPATH
  truncate(sr.originalShader, MAX_QPATH);
  truncate(sr.currentShader, MAX_QPATH);

  // get the indices as well
  sr.i1 = getShaderIndex(oldShader, true);
  sr.i2 = getShaderIndex(newShader, true);

  remapQueue.insert(
      static_cast<int16_t>(shaderRemaps.size() / MAX_SHADERSTATES_PER_INDEX));
  shaderRemaps.emplace_back(sr);
}

void ShaderConfigHandler::updateShaderState() {
  if (remapQueue.empty()) {
    return;
  }

  for (const auto &slot : remapQueue) {
    std::string state;

    const int32_t start = slot * MAX_SHADERSTATES_PER_INDEX;
    const int32_t end = std::min(start + MAX_SHADERSTATES_PER_INDEX,
                                 static_cast<int32_t>(shaderRemaps.size()));

    for (int32_t i = start; i < end; i++) {
      state += buildShaderStateString(i);
    }

    trap_SetConfigstring(CS_SHADERSTATE + slot, state.c_str());
  }

  remapQueue.clear();
}

std::string ShaderConfigHandler::buildShaderStateString(const int32_t index) {
  assert(index < static_cast<int32_t>(shaderRemaps.size()));

  const auto &sr = shaderRemaps[index];
  assert(sr.i1 != 0 && sr.i2 != 0);

  return StringUtils::format("%i=%i:%5.2f@", sr.i1, sr.i2, sr.timeOffset);
}
} // namespace ETJump
