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

#include "etj_shader_index_handler.h"
#include "etj_printer.h"

namespace ETJump {
int32_t ShaderIndexHandler::getShaderIndexExt(const std::string_view name) {
  for (int32_t i = 0; i < shaderSets.size(); i++) {
    for (const auto &[index, shader] : shaderSets[i]) {
      if (shader == name) {
        return index;
      }
    }

    // set is full, try next one
    if (shaderSets[i].size() == MAX_CS_SHADERS) {
      continue;
    }

    // store the shader in the next free index
    const int32_t index =
        ((i + 1) * MAX_CS_SHADERS) + static_cast<int32_t>(shaderSets[i].size());
    shaderSets[i][index] = name;

    G_DPrintf(
        "%s: registered shader '%s' to extended shader index %i [%i:%i]\n",
        __func__, std::string(name).c_str(), index, i,
        static_cast<int32_t>(shaderSets[i].size() - 1));

    return index;
  }

  // all sets were full, bail
  G_Error("%s: extended shader index overflown, too many shaders registered "
          "(max %i)!\n",
          __func__, MAX_SHADER_INDEX - 1);
}

void ShaderIndexHandler::sendShadersExt(const int32_t clientNum) const {
  // we don't need to send the index for the set that a shader belongs to,
  // as the client can determine that based off the shader index number
  const std::string cmdPrefix = "extShaderIndex";

  for (const auto &shaderSet : shaderSets) {
    if (shaderSets.empty()) {
      break;
    }

    std::string cmd = cmdPrefix;

    for (const auto &[index, shader] : shaderSet) {
      const std::string current = " " + std::to_string(index) + "|" + shader;

      // send it if we're about to run over the max message size
      if (cmd.length() + current.length() > BYTES_PER_PACKET) {
        Printer::command(clientNum, cmd);
        cmd = cmdPrefix;
      }

      cmd += current;
    }

    // send the rest if any are remaining
    if (cmd.length() > cmdPrefix.length()) {
      Printer::command(clientNum, cmd);
    }
  }
}
} // namespace ETJump
