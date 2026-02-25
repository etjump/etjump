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

#include "etj_keyset_drawer.h"
#include "etj_local.h"
#include "etj_utilities.h"

namespace ETJump {
KeySetDrawer::KeySetDrawer(const std::vector<KeyShader> &keyShaders,
                           const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : keyShaders(keyShaders), cvarUpdate(cvarUpdate) {
  initAttrs();
  initListeners();
}

KeySetDrawer::~KeySetDrawer() {
  cvarUpdate->unsubscribe(&etj_keysColor);
  cvarUpdate->unsubscribe(&etj_keysSize);
  cvarUpdate->unsubscribe(&etj_keysX);
  cvarUpdate->unsubscribe(&etj_keysY);
  cvarUpdate->unsubscribe(&etj_keysShadow);
}

void KeySetDrawer::initListeners() {
  cvarUpdate->subscribe(&etj_keysColor, [this](const vmCvar_t *cvar) {
    updateKeysColor(cvar->string);
  });

  cvarUpdate->subscribe(&etj_keysSize,
                        [this](const vmCvar_t *cvar) { updateKeysSize(cvar); });

  cvarUpdate->subscribe(&etj_keysX, [this](const vmCvar_t *) {
    updateKeysOrigin(etj_keysX.value, etj_keysY.value);
  });

  cvarUpdate->subscribe(&etj_keysY, [this](const vmCvar_t *) {
    updateKeysOrigin(etj_keysX.value, etj_keysY.value);
  });

  cvarUpdate->subscribe(&etj_keysShadow, [this](const vmCvar_t *cvar) {
    updateKeysShadow(cvar->integer);
  });
}

void KeySetDrawer::initAttrs() {
  updateKeysColor(etj_keysColor.string);
  updateKeysSize(&etj_keysSize);
  updateKeysOrigin(etj_keysX.value, etj_keysY.value);
  updateKeysShadow(etj_keysShadow.integer);
  vec4_t shadowColor{0.0f, 0.0f, 0.0f, 1.0f};
  updateKeysShadowColor(shadowColor);
}

void KeySetDrawer::updateKeysColor(const char *str) {
  cgame.utils.colorParser->parseColorString(str, attrs.color);
}

void KeySetDrawer::updateKeysSize(const vmCvar_t *cvar) {
  attrs.size = CvarValueParser::parse<CvarValue::Size>(*cvar, 0, 256);
}

void KeySetDrawer::updateKeysOrigin(float x, float y) {
  attrs.origin.x = ETJump_AdjustPosition(std::clamp(x, 0.f, 640.f));
  attrs.origin.y = std::clamp(y, 0.f, 480.f);
}

void KeySetDrawer::updateKeysShadow(bool shouldDrawShadow) {
  attrs.shouldDrawShadow = shouldDrawShadow;
}

void KeySetDrawer::updateKeysShadowColor(const vec4_t shadowColor) {
  Vector4Copy(shadowColor, attrs.shadowColor);
}

void KeySetDrawer::render() const {
  if (attrs.size.x == 0 || attrs.size.y == 0) {
    return;
  }

  for (auto i = 0; i < static_cast<int>(keyShaders.size()); i++) {
    const auto &shader = keyShaders[i];
    if (isKeyPressed(shader.key)) {
      drawPressShader(shader.press, i);
    } else {
      drawReleaseShader(shader.release, i);
    }
  }
}

void KeySetDrawer::drawPressShader(qhandle_t shader, int position) const {
  drawShader(shader, position);
}

void KeySetDrawer::drawReleaseShader(qhandle_t shader, int position) const {
  drawShader(shader, position);
}

void KeySetDrawer::drawShader(qhandle_t shader, int position) const {
  if (!shader) {
    return;
  }

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

  drawPic(x, y, sizeX, sizeY, shader, color, shadowColor);
}

int KeySetDrawer::isKeyPressed(KeyNames key) {
  const auto ps = getValidPlayerState();
  switch (key) {
    case KeyNames::Forward:
      return ps->stats[STAT_USERCMD_MOVE] & UMOVE_FORWARD;
    case KeyNames::Backward:
      return ps->stats[STAT_USERCMD_MOVE] & UMOVE_BACKWARD;
    case KeyNames::Right:
      return ps->stats[STAT_USERCMD_MOVE] & UMOVE_RIGHT;
    case KeyNames::Left:
      return ps->stats[STAT_USERCMD_MOVE] & UMOVE_LEFT;
    case KeyNames::Jump:
      return ps->stats[STAT_USERCMD_MOVE] & UMOVE_UP;
    case KeyNames::Crouch:
      return ps->stats[STAT_USERCMD_MOVE] & UMOVE_DOWN;
    case KeyNames::Sprint:
      return ps->stats[STAT_USERCMD_BUTTONS] & BUTTON_SPRINT << 8;
    case KeyNames::Prone:
      return ps->stats[STAT_USERCMD_BUTTONS] & WBUTTON_PRONE;
    case KeyNames::LeanRight:
      return ps->stats[STAT_USERCMD_BUTTONS] & WBUTTON_LEANRIGHT;
    case KeyNames::LeanLeft:
      return ps->stats[STAT_USERCMD_BUTTONS] & WBUTTON_LEANLEFT;
    case KeyNames::Walk:
      return ps->stats[STAT_USERCMD_BUTTONS] & BUTTON_WALKING << 8;
    case KeyNames::Talk:
      return ps->stats[STAT_USERCMD_BUTTONS] & BUTTON_TALK << 8;
    case KeyNames::Activate:
      return ps->stats[STAT_USERCMD_BUTTONS] & BUTTON_ACTIVATE << 8;
    case KeyNames::Attack:
      return ps->stats[STAT_USERCMD_BUTTONS] & BUTTON_ATTACK << 8;
    case KeyNames::Attack2:
      return ps->stats[STAT_USERCMD_BUTTONS] & WBUTTON_ATTACK2;
    case KeyNames::Reload:
      return ps->stats[STAT_USERCMD_BUTTONS] & WBUTTON_RELOAD;
    case KeyNames::Zoom:
      return ps->stats[STAT_USERCMD_BUTTONS] & WBUTTON_ZOOM;
    default:
      return false;
  }
}

std::string KeySetDrawer::keyNameToString(KeyNames keyName) {
  switch (keyName) {
    case KeyNames::Forward:
      return "forward";
    case KeyNames::Backward:
      return "backward";
    case KeyNames::Right:
      return "right";
    case KeyNames::Left:
      return "left";
    case KeyNames::Jump:
      return "jump";
    case KeyNames::Crouch:
      return "crouch";
    case KeyNames::Sprint:
      return "sprint";
    case KeyNames::Prone:
      return "prone";
    case KeyNames::LeanRight:
      return "learnright";
    case KeyNames::LeanLeft:
      return "leanleft";
    case KeyNames::Walk:
      return "walk";
    case KeyNames::Talk:
      return "talk";
    case KeyNames::Activate:
      return "activate";
    case KeyNames::Attack:
      return "attack";
    case KeyNames::Attack2:
      return "attack2";
    case KeyNames::Reload:
      return "reload";
    case KeyNames::Zoom:
      return "zoom";
    default:
      return "";
  }
}
} // namespace ETJump
