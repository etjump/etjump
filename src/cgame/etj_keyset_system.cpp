/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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

#include "etj_keyset_system.h"
#include "etj_cvar_master_drawer.h"
#include "etj_keyset_drawer.h"
#include "etj_keyset_keybind_drawer.h"
#include "../game/etj_string_utilities.h"

ETJump::KeySetSystem::KeySetSystem(const vmCvar_t &controlCvar)
    : keySetMasterDrawer(controlCvar) {}

ETJump::KeySetSystem::~KeySetSystem() {}

void ETJump::KeySetSystem::addSet(const std::string &keySetName) {
  keySetMasterDrawer.push(new KeySetDrawer(createKeyPressSet(keySetName)));
}

void ETJump::KeySetSystem::addKeyBindSet(const std::string &keySetName) {
  keySetMasterDrawer.push(new KeySetKeyBindDrawer(
      createKeyPressSet(keySetName), createKeyBindSet(keySetName)));
}

bool ETJump::KeySetSystem::beforeRender() {
  return keySetMasterDrawer.beforeRender();
}

void ETJump::KeySetSystem::render() const {
  if (canSkipDraw()) {
    return;
  }
  keySetMasterDrawer.render();
}

std::vector<ETJump::KeySetDrawer::KeyShader>
ETJump::KeySetSystem::createKeyPressSet(const std::string &keySetName) {
  std::vector<KeySetDrawer::KeyShader> keyPressSet;
  for (auto &&keyName : keyLayout) {
    keyPressSet.push_back(
        {keyName,
         registerKeySetShader(keySetName,
                              KeySetDrawer::keyNameToString(keyName)),
         0});
  }
  return keyPressSet;
}

std::map<int, qhandle_t>
ETJump::KeySetSystem::createKeyBindSet(const std::string &keySetName) {
  return {
      {K_ENTER, registerKeySetShader(keySetName, "enter")},
      {K_SHIFT, registerKeySetShader(keySetName, "shift")},
      {K_CAPSLOCK, registerKeySetShader(keySetName, "capslock")},
      {K_CTRL, registerKeySetShader(keySetName, "control")},
      {K_TAB, registerKeySetShader(keySetName, "tab")},
      {K_ALT, registerKeySetShader(keySetName, "alt")},
      {K_BACKSPACE, registerKeySetShader(keySetName, "backspace")},
      {K_DEL, registerKeySetShader(keySetName, "delete")},
      {K_DOWNARROW, registerKeySetShader(keySetName, "downarrow")},
      {K_UPARROW, registerKeySetShader(keySetName, "uparrow")},
      {K_RIGHTARROW, registerKeySetShader(keySetName, "rightarrow")},
      {K_LEFTARROW, registerKeySetShader(keySetName, "leftarrow")},
      {K_PGDN, registerKeySetShader(keySetName, "pagedown")},
      {K_PGUP, registerKeySetShader(keySetName, "pageup")},
      {K_HOME, registerKeySetShader(keySetName, "home")},
      {K_END, registerKeySetShader(keySetName, "end")},
      {K_ESCAPE, registerKeySetShader(keySetName, "escape")},
      {K_SPACE, registerKeySetShader(keySetName, "space")},
      {K_INS, registerKeySetShader(keySetName, "insert")},
      {K_PAUSE, registerKeySetShader(keySetName, "pause")},
      {K_MWHEELDOWN, registerKeySetShader(keySetName, "mwheeldown")},
      {K_MWHEELUP, registerKeySetShader(keySetName, "mwheelup")},
  };
}

qhandle_t
ETJump::KeySetSystem::registerKeySetShader(const std::string &keySetName,
                                           const std::string &keyName) {
  auto path = createKeyPressSetShaderPath(keySetName, keyName);
  return registerShaderNoMip(path);
}

std::string
ETJump::KeySetSystem::createKeyPressSetShaderPath(const std::string &keySetName,
                                                  const std::string &keyName) {
  if (keyName.size() == 0) {
    return "";
  }
  return stringFormat("gfx/%s/key_%s_pressed", keySetName, keyName);
}

qhandle_t
ETJump::KeySetSystem::registerShaderNoMip(const std::string &shaderPath) {
  if (shaderPath.size() == 0) {
    return 0;
  }
  return trap_R_RegisterShaderNoMip(shaderPath.c_str());
}

bool ETJump::KeySetSystem::canSkipDraw() const {
  return cg.showScores || cg.scoreFadeTime + FADE_TIME > cg.time;
}
