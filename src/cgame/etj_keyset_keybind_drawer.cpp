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

#include "etj_keyset_keybind_drawer.h"
#include "../game/etj_string_utilities.h"

ETJump::KeySetKeyBindDrawer::KeySetKeyBindDrawer(
    const std::vector<KeyShader> &keyShaders,
    const std::map<int, qhandle_t> controlShaders)
    : KeySetDrawer(keyShaders), controlShaders(controlShaders) {}

void ETJump::KeySetKeyBindDrawer::drawReleaseShader(qhandle_t shader,
                                                    int position) const {}

void ETJump::KeySetKeyBindDrawer::drawPressShader(qhandle_t shader,
                                                  int position) const {
  if (!shader) {
    return;
  }

  // get command bind key name
  const float sizeX = attrs.size.x / 3;
  const float sizeY = attrs.size.y / 3;
  const float centerOffsetX = attrs.size.x / 2;
  const float centerOffsetY = attrs.size.y / 2;
  const auto pos = calcGridPosition<3>(sizeX, sizeY, position);
  const float x = attrs.origin.x + pos.x - centerOffsetX;
  const float y = attrs.origin.y + pos.y - centerOffsetY;
  const vec_t *color = attrs.color;
  const vec_t *shadowColor =
      attrs.shouldDrawShadow ? attrs.shadowColor : nullptr;

  // key code handling
  const auto keyShader = keyShaders[position];
  auto command = keyNameToCommand(keyShader.key);
  auto keyCode = getKeyCodeForName(command);
  keyCode = checkKeyCodeRemap(keyCode);
  const auto keyCodeShader = checkIfKeyCodeHasShader(keyCode);

  // background
  drawPic(x, y, sizeX, sizeY, shader, color, shadowColor);

  if (keyCodeShader) {
    drawPic(x, y, sizeX, sizeY, keyCodeShader, color, shadowColor);
  } else {
    auto binding = ETJump::StringUtil::toUpperCase(getKeyCodeBinding(keyCode));
    // factor = 16 / ... = 0.20
    // size / factor;
    auto bindWidth = DrawStringWidth(binding.c_str(), 0.2f);
    auto bindHeight = DrawStringHeight(binding.c_str(), 0.2f);
    auto charOffsetX = (sizeX - bindWidth) / 2;
    auto charOffsetY = (sizeY + bindHeight + 1) / 2;
    DrawString(x + charOffsetX, y + charOffsetY, 0.20f, 0.20f, color, qfalse,
               binding.c_str(), 0, 0);
  }
}

std::string ETJump::KeySetKeyBindDrawer::keyNameToCommand(KeyNames keyName) {
  switch (keyName) {
    case KeyNames::Forward:
      return "+forward";
    case KeyNames::Backward:
      return "+back";
    case KeyNames::Right:
      return "+moveright";
    case KeyNames::Left:
      return "+moveleft";
    case KeyNames::Jump:
      return "+moveup";
    case KeyNames::Crouch:
      return "+movedown";
    case KeyNames::Sprint:
      return "+sprint";
    case KeyNames::Prone:
      return "+prone";
    case KeyNames::LeanRight:
      return "+leanright";
    case KeyNames::LeanLeft:
      return "+leanleft";
    case KeyNames::Walk:
      return "+speed";
    case KeyNames::Talk:
      return "";
    case KeyNames::Activate:
      return "+activate";
    case KeyNames::Attack:
      return "+attack";
    case KeyNames::Attack2:
      return "+attack2";
    case KeyNames::Reload:
      return "+reload";
    case KeyNames::Zoom:
      return "+zoom";
    default:
      return "";
  }
}

int ETJump::KeySetKeyBindDrawer::getKeyCodeForName(std::string &name) {
  if (name.size()) {
    int b1, b2;
    trap_Key_KeysForBinding(name.c_str(), &b1, &b2);
    if (b1 != -1) {
      return b1;
    }
  }
  return 0;
}

std::string ETJump::KeySetKeyBindDrawer::getKeyCodeBinding(int keyCode) {
  auto binding = getKeyCodeShortBinding(keyCode);
  if (!binding.size()) {
    binding = getKeyCodeFullBinding(keyCode);
  }
  return binding;
}

std::string ETJump::KeySetKeyBindDrawer::getKeyCodeFullBinding(int keyCode) {
  if (keyCode) {
    char buf[32];
    trap_Key_KeynumToStringBuf(keyCode, buf, 32);
    return buf;
  }
  return "?";
}

std::string ETJump::KeySetKeyBindDrawer::getKeyCodeShortBinding(int keyCode) {
  switch (keyCode) {
    case K_MOUSE1:
      return "m1";
    case K_MOUSE2:
      return "m2";
    case K_MOUSE3:
      return "m3";
    case K_MOUSE4:
      return "m4";
    case K_MOUSE5:
      return "m5";
    default:
      return "";
  }
}

int ETJump::KeySetKeyBindDrawer::checkKeyCodeRemap(int keyCode) const {
  switch (keyCode) {
    case K_KP_HOME:
      return K_HOME;
    case K_KP_UPARROW:
      return K_UPARROW;
    case K_KP_PGUP:
      return K_PGUP;
    case K_KP_LEFTARROW:
      return K_LEFTARROW;
    case K_KP_5:
      return 0x35; // '5'
    case K_KP_RIGHTARROW:
      return K_RIGHTARROW;
    case K_KP_END:
      return K_END;
    case K_KP_DOWNARROW:
      return K_DOWNARROW;
    case K_KP_PGDN:
      return K_PGDN;
    case K_KP_ENTER:
      return K_ENTER;
    case K_KP_INS:
      return K_INS;
    case K_KP_DEL:
      return K_DEL;
    case K_KP_SLASH:
      return 0x2f; // '/'
    case K_KP_MINUS:
      return 0x2d; // '-'
    case K_KP_PLUS:
      return 0x2b; // '+'
    case K_KP_NUMLOCK:
      return 0x3f; // '?'
    case K_KP_STAR:
      return 0x2a; // '*'
    case K_KP_EQUALS:
      return 0x3d; // '='
    case K_COMMAND:
      return 0x3f; // '?'
    default:
      break;
  }
  // joy, aux
  if (keyCode >= 185 && keyCode <= 232) {
    return 0x3f; // '?'
  }
  return keyCode;
}

qhandle_t
ETJump::KeySetKeyBindDrawer::checkIfKeyCodeHasShader(int keyCode) const {
  const auto it = controlShaders.find(keyCode);
  if (it != controlShaders.end()) {
    return it->second;
  }
  return 0;
}
