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

#pragma once
#include <string>
#include <vector>
#include "etj_keyset_drawer.h"
#include "cg_local.h"
#include <map>
#include "etj_cvar_master_drawer.h"
#include "etj_irenderable.h"

namespace ETJump {
class KeySetSystem : public IRenderable {
  // key set layout
  std::vector<KeySetDrawer::KeyNames> const keyLayout{
      KeySetDrawer::KeyNames::Sprint, KeySetDrawer::KeyNames::Forward,
      KeySetDrawer::KeyNames::Jump,   KeySetDrawer::KeyNames::Left,
      KeySetDrawer::KeyNames::Empty,  KeySetDrawer::KeyNames::Right,
      KeySetDrawer::KeyNames::Prone,  KeySetDrawer::KeyNames::Backward,
      KeySetDrawer::KeyNames::Crouch,
  };
  CvarBasedMasterDrawer keySetMasterDrawer;

  std::vector<KeySetDrawer::KeyShader>
  createKeyPressSet(const std::string &keySetName);
  std::map<int, qhandle_t> createKeyBindSet(const std::string &keySetName);
  qhandle_t registerKeySetShader(const std::string &keySetName,
                                 const std::string &keyName);
  std::string createKeyPressSetShaderPath(const std::string &keySetName,
                                          const std::string &keyName);
  qhandle_t registerShaderNoMip(const std::string &shaderName);
  bool canSkipDraw() const;

public:
  KeySetSystem(const vmCvar_t &cvar);
  ~KeySetSystem();
  void addSet(const std::string &keySetName);
  void addKeyBindSet(const std::string &keySetName);
  bool beforeRender() override;
  void render() const override;
};
} // namespace ETJump
