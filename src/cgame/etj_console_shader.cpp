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

#include "etj_console_shader.h"
#include "cg_local.h"
#include "etj_color_parser.h"
#include "etj_utilities.h"

#include "../game/etj_string_utilities.h"

namespace ETJump {
ConsoleShader::ConsoleShader() {
  auto shader = createBackground();
  trap_R_LoadDynamicShader(shaderName, shader.c_str());
  // once a dynamic shader is registered, any changes to it will have no effect
  trap_R_RegisterShader(shaderName);
  trap_R_RemapShader("console-16bit", shaderName, "0");
}

std::string ConsoleShader::createBackground() {
  if (etj_consoleShader.integer) {
    return createTexturedBackground();
  }

  return createSolidBackground();
}

std::string ConsoleShader::createTexturedBackground() {
  auto alphaGen = stringFormat("alphaGen const %f", etj_consoleAlpha.value);

  return composeShader(shaderName, {"nopicmip"},
                       {{"map textures/skies_sd/wurzburg_clouds.tga",
                         "blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA",
                         alphaGen.c_str(), "rgbGen identityLighting",
                         "tcMod scale 1.5 .75", "tcMod scroll 0.01 -0.005"},
                        {
                            "map textures/skies_sd/wurzburg_clouds.tga",
                            "blendFunc GL_DST_COLOR GL_ONE",
                            "rgbGen identityLighting",
                            "tcMod scale 1.25 .85",
                            "tcMod transform 0 1 1 0 0 0",
                            "tcMod scroll 0.03 -0.015",
                        },
                        {
                            "clampmap textures/skies_sd/colditz_mask.tga",
                            "blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA",
                            alphaGen.c_str(),
                            "rgbGen identityLighting",
                            "tcMod scale 0.85 0.5",
                            "tcMod transform 1 0 0 1 -0.075 0.42",
                        }});
}

std::string ConsoleShader::createSolidBackground() {
  vec4_t bg;
  cgame.utils.colorParser->parseColorString(etj_consoleColor.string, bg);
  auto alphaGen = stringFormat("alphaGen const %f", etj_consoleAlpha.value);
  auto colorGen =
      stringFormat("rgbGen const ( %f %f %f )", bg[0], bg[1], bg[2]);

  return composeShader(shaderName, {"nopicmip"},
                       {{
                           "map $whiteimage",
                           "blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA",
                           alphaGen.c_str(),
                           colorGen.c_str(),
                       }});
}

ConsoleShader::~ConsoleShader() {
  trap_R_RemapShader("console-16bit", "console-16bit", "0");
}
} // namespace ETJump
