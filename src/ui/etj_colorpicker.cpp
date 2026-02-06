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

#include "etj_colorpicker.h"

#include "../cgame/etj_utilities.h"
#include "../game/etj_string_utilities.h"

extern displayContextDef_t *DC;

namespace ETJump {
static std::unordered_map<std::string, float> colorPickerValues{
    {COLOR_PICKER_H, 0.0f},   {COLOR_PICKER_S, 100.0f},
    {COLOR_PICKER_V, 100.0f}, {COLOR_PICKER_R, 255.0f},
    {COLOR_PICKER_G, 0.0f},   {COLOR_PICKER_B, 0.0f},
    {COLOR_PICKER_A, 1.0f},
};

ColorPicker::ColorPicker() {
  Vector4Set(pickerColor, 0.75f, 0.75f, 0.75f, 0.75f);
  normalizedRGBSliders = false;
}

void ColorPicker::drawPreviewOld(const rectDef_t *rect) const {
  DC->fillRect(rect->x, rect->y, rect->w, rect->h, oldRGB);
}

void ColorPicker::drawPreviewNew(const rectDef_t *rect) {
  if (!normalizedRGBSliders) {
    Vector4Copy(fullRGB, normalizedRGB);
    setRGBNormalized(normalizedRGB, colorPickerValues[COLOR_PICKER_R],
                     colorPickerValues[COLOR_PICKER_G],
                     colorPickerValues[COLOR_PICKER_B],
                     colorPickerValues[COLOR_PICKER_A]);
  }

  DC->fillRect(rect->x, rect->y, rect->w, rect->h, normalizedRGB);
}

void ColorPicker::drawColorPicker(const rectDef_t *rect) {
  // we want to use default S/V/A values here since we're just drawing
  // a single color and using a mask image to draw S/V effects
  // alpha is also only displayed in the preview boxes, not here
  setHSV(pickerHSV, colorPickerValues[COLOR_PICKER_H], 100, 100, 1.0f);
  BG_HSVtoRGB(pickerHSV, pickerRGB, true);

  // color selector
  DC->fillRect(rect->x, rect->y, rect->w, rect->h, pickerRGB);
  DC->drawHandlePic(rect->x, rect->y, rect->w, rect->h,
                    DC->Assets.colorPickerMask);

  // color picker "crosshair"
  DC->fillRect(rect->x + colorPickerValues[COLOR_PICKER_S], rect->y, 1, rect->h,
               pickerColor);
  DC->fillRect(rect->x, rect->y + (rect->h - colorPickerValues[COLOR_PICKER_V]),
               rect->w, 1, pickerColor);
}

void ColorPicker::cvarToColorPickerState(const std::string &cvar) {
  if (cvar.empty()) {
    Com_Printf(S_COLOR_YELLOW "%s: cannot set slider state from empty cvar\n",
               __func__);
    return;
  }

  currentCvar = cvar;
  char buf[MAX_CVAR_VALUE_STRING];
  DC->getCVarString(cvar.c_str(), buf, sizeof(buf));
  currentCvarOldValue = buf;

  // this can potentially be a bit wasteful if the color is already
  // in 0-255 range, but I'd rather not complicate the logic around that
  parseColorString(currentCvarOldValue, normalizedRGB);

  Vector4Copy(normalizedRGB, oldRGB);
  Vector4Copy(normalizedRGB, pickerRGB);

  VectorScale(normalizedRGB, 255, fullRGB);
  BG_RGBtoHSV(fullRGB, HSV);

  updateHSVSliderState(HSV);
  updateRGBSliderState(fullRGB);
  colorPickerValues[COLOR_PICKER_A] = normalizedRGB[3];
}

void ColorPicker::colorPickerStateToCvar(const bool reset) const {
  if (currentCvar.empty()) {
    Com_Printf(S_COLOR_YELLOW "%s: cannot set slider state to empty cvar\n",
               __func__);
    return;
  }

  if (reset) {
    DC->setCVar(currentCvar.c_str(), currentCvarOldValue.c_str());
  } else {
    const std::string &value =
        stringFormat("%f %f %f %f", normalizedRGB[0], normalizedRGB[1],
                     normalizedRGB[2], normalizedRGB[3]);
    DC->setCVar(currentCvar.c_str(), value.c_str());
  }
}

void ColorPicker::resetColorPickerState() {
  Vector4Copy(oldRGB, normalizedRGB);
  VectorScale(normalizedRGB, 255, fullRGB);
  BG_RGBtoHSV(fullRGB, HSV);

  updateHSVSliderState(HSV);
  updateRGBSliderState(fullRGB);
  colorPickerValues[COLOR_PICKER_A] = normalizedRGB[3];

  colorPickerStateToCvar(true);
}

inline void ColorPicker::setHSV(vec4_t hsv, const float h, const float s,
                                const float v, const float a) {
  Vector4Set(hsv, std::clamp(h, 0.0f, 360.0f), std::clamp(s, 0.0f, 100.0f),
             std::clamp(v, 0.0f, 100.0f), std::clamp(a, 0.0f, 1.0f));
}

inline void ColorPicker::setRGB(vec4_t rgb, const float r, const float g,
                                const float b, const float a) {
  Vector4Set(rgb, std::clamp(r, 0.0f, 255.0f), std::clamp(g, 0.0f, 255.0f),
             std::clamp(b, 0.0f, 255.0f), std::clamp(a, 0.0f, 1.0f));
}

inline void ColorPicker::setRGBNormalized(vec4_t rgb, const float r,
                                          const float g, const float b,
                                          const float a) {
  Vector4Set(rgb, std::clamp(r / 255, 0.0f, 1.0f),
             std::clamp(g / 255, 0.0f, 1.0f), std::clamp(b / 255, 0.0f, 1.0f),
             std::clamp(a, 0.0f, 1.0f));
}

void ColorPicker::updateRGBSliderState(const vec4_t rgb) const {
  if (normalizedRGBSliders) {
    colorPickerValues[COLOR_PICKER_R] = rgb[0] / 255;
    colorPickerValues[COLOR_PICKER_G] = rgb[1] / 255;
    colorPickerValues[COLOR_PICKER_B] = rgb[2] / 255;
  } else {
    colorPickerValues[COLOR_PICKER_R] = rgb[0];
    colorPickerValues[COLOR_PICKER_G] = rgb[1];
    colorPickerValues[COLOR_PICKER_B] = rgb[2];
  }
}

void ColorPicker::updateHSVSliderState(const vec4_t hsv) {
  colorPickerValues[COLOR_PICKER_H] = hsv[0];
  colorPickerValues[COLOR_PICKER_S] = hsv[1];
  colorPickerValues[COLOR_PICKER_V] = hsv[2];
}

void ColorPicker::updateSliderState(const itemDef_t *item) {
  // note: regardless of which slider is being updated, 'normalizedRGB' must be
  // updated as that's what 'colorPickerStateToCvar' uses to set the cvar value
  switch (item->colorSliderData.colorType) {
    case COLOR_HSV:
      setHSV(HSV, colorPickerValues[COLOR_PICKER_H],
             colorPickerValues[COLOR_PICKER_S],
             colorPickerValues[COLOR_PICKER_V],
             colorPickerValues[COLOR_PICKER_A]);
      BG_HSVtoRGB(HSV, fullRGB, false);
      setRGBNormalized(normalizedRGB, fullRGB[0], fullRGB[1], fullRGB[2],
                       fullRGB[3]);
      updateRGBSliderState(fullRGB);
      break;
    case COLOR_RGB:
    case COLOR_ALPHA:
      if (normalizedRGBSliders) {
        setRGB(normalizedRGB, colorPickerValues[COLOR_PICKER_R],
               colorPickerValues[COLOR_PICKER_G],
               colorPickerValues[COLOR_PICKER_B],
               colorPickerValues[COLOR_PICKER_A]);
        Vector4Scale(normalizedRGB, 255, fullRGB);
      } else {
        setRGB(fullRGB, colorPickerValues[COLOR_PICKER_R],
               colorPickerValues[COLOR_PICKER_G],
               colorPickerValues[COLOR_PICKER_B],
               colorPickerValues[COLOR_PICKER_A]);
        setRGBNormalized(normalizedRGB, colorPickerValues[COLOR_PICKER_R],
                         colorPickerValues[COLOR_PICKER_G],
                         colorPickerValues[COLOR_PICKER_B],
                         colorPickerValues[COLOR_PICKER_A]);
      }

      BG_RGBtoHSV(fullRGB, HSV);
      updateHSVSliderState(HSV);
      break;
    case COLOR_UNKNOWN:
    default:
      break;
  }

  colorPickerStateToCvar(false);
}

const char *ColorPicker::getColorSliderString(const int handle) {
  const char *colorVar = nullptr;

  if (!PC_String_Parse(handle, &colorVar)) {
    return nullptr;
  }

  for (const auto &str : colorPickerValues) {
    if (str.first == colorVar) {
      return colorVar;
    }
  }

  Com_Printf(S_COLOR_YELLOW "%s: invalid color slider string '%s'\n", __func__,
             colorVar);
  return nullptr;
}

void ColorPicker::setColorSliderType(itemDef_t *item) {
  const std::string &colorVar = item->colorSliderData.colorVar;

  if (colorVar == COLOR_PICKER_H || colorVar == COLOR_PICKER_S ||
      colorVar == COLOR_PICKER_V) {
    item->colorSliderData.colorType = COLOR_HSV;
  } else if (colorVar == COLOR_PICKER_R || colorVar == COLOR_PICKER_G ||
             colorVar == COLOR_PICKER_B) {
    item->colorSliderData.colorType = COLOR_RGB;
  } else if (colorVar == COLOR_PICKER_A) {
    item->colorSliderData.colorType = COLOR_ALPHA;
  } else {
    Com_Printf(S_COLOR_YELLOW "%s: unable to set color slider type for %s\n",
               __func__, item->window.name ? item->window.name : "(unnamed)");
    item->colorSliderData.colorType = COLOR_UNKNOWN;
  }
}

float ColorPicker::getColorSliderValue(const std::string &colorVar) {
  const auto it = colorPickerValues.find(colorVar);

  if (it != colorPickerValues.end()) {
    return it->second;
  }

  Com_Printf(S_COLOR_YELLOW "%s: unable to find color slider value for '%s'\n",
             __func__, colorVar.c_str());
  return 0.0f;
}

void ColorPicker::setColorSliderValue(const std::string &colorVar,
                                      const float value) {
  colorPickerValues[colorVar] = value;
}

void ColorPicker::toggleRGBSliderValues() {
  const menuDef_t *menu = Menus_FindByName(COLOR_PICKER_MENU);

  // this should never happen, we already have the menu open...
  if (menu == nullptr) {
    Com_Printf(S_COLOR_YELLOW "%s: unable to find color picker menu '%s'\n",
               __func__, COLOR_PICKER_MENU);
    return;
  }

  bool redFound = false;
  bool greenFound = false;
  bool blueFound = false;

  // flip flop
  normalizedRGBSliders = !normalizedRGBSliders;

  const auto swapLabelType = [&](itemDef_t *item) {
    if (normalizedRGBSliders) {
      item->window.flags &= ~WINDOW_TEXTASINT;
      item->window.flags |= WINDOW_TEXTASFLOAT;
    } else {
      item->window.flags &= ~WINDOW_TEXTASFLOAT;
      item->window.flags |= WINDOW_TEXTASINT;
    }
  };

  for (int i = 0; i < menu->itemCount; i++) {
    itemDef_t *item = menu->items[i];

    if (!item->colorSliderData.colorVar) {
      continue;
    }

    const auto editDef = static_cast<editFieldDef_t *>(item->typeData);
    const std::string &colorVar = item->colorSliderData.colorVar;

    if (colorVar == COLOR_PICKER_R) {
      if (item->type == ITEM_TYPE_TEXT) {
        swapLabelType(item);
      } else {
        editDef->defVal = normalizedRGBSliders ? 1.0f : 255.0f;
        editDef->maxVal = normalizedRGBSliders ? 1.0f : 255.0f;
        editDef->step = normalizedRGBSliders ? 0.01f : 1.0f;
        redFound = true;
      }
    } else if (colorVar == COLOR_PICKER_G) {
      if (item->type == ITEM_TYPE_TEXT) {
        swapLabelType(item);
      } else {
        editDef->defVal = 0.0f;
        editDef->maxVal = normalizedRGBSliders ? 1.0f : 255.0f;
        editDef->step = normalizedRGBSliders ? 0.01f : 1.0f;
        greenFound = true;
      }
    } else if (colorVar == COLOR_PICKER_B) {
      if (item->type == ITEM_TYPE_TEXT) {
        swapLabelType(item);
      } else {
        editDef->defVal = 0.0f;
        editDef->maxVal = normalizedRGBSliders ? 1.0f : 255.0f;
        editDef->step = normalizedRGBSliders ? 0.01f : 1.0f;
        blueFound = true;
      }
    }
  }

  // we should always find these but just in case
  if (redFound && greenFound && blueFound) {
    updateRGBSliderState(fullRGB);
  } else {
    Com_Printf(S_COLOR_YELLOW
               "%s: unable to find color sliders for conversion\n",
               __func__);
  }
}

void ColorPicker::colorPickerDragFunc(const itemDef_t *item,
                                      const float cursorX, const float cursorY,
                                      const int key) {
  rectDef_t rect = item->window.rect;
  shrinkRectForColorPicker(rect);

  // mouse2 adjusts only saturation, mouse3 only value
  if (key != K_MOUSE2) {
    const float y = std::clamp(cursorY, rect.y, rect.y + rect.h);
    // value increases as Y pos decreases, so invert the movement
    setColorSliderValue(COLOR_PICKER_V, rect.h - (y - rect.y));
  }

  if (key != K_MOUSE3) {
    const float x = std::clamp(cursorX, rect.x, rect.x + rect.w);
    setColorSliderValue(COLOR_PICKER_S, x - rect.x);
  }

  setHSV(HSV, colorPickerValues[COLOR_PICKER_H],
         colorPickerValues[COLOR_PICKER_S], colorPickerValues[COLOR_PICKER_V],
         colorPickerValues[COLOR_PICKER_A]);
  BG_HSVtoRGB(HSV, normalizedRGB, true);
  Vector4Scale(normalizedRGB, 255, fullRGB);
  updateRGBSliderState(fullRGB);

  colorPickerStateToCvar(false);
}

void ColorPicker::shrinkRectForColorPicker(rectDef_t &rect) {
  rect.x += 2;
  rect.y += 2;
  rect.w -= 4;
  rect.h -= 4;
}

bool ColorPicker::RGBSlidersAreNormalized() const {
  return normalizedRGBSliders;
}
} // namespace ETJump
