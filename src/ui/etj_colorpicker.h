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

#include "ui_shared.h"

namespace ETJump {
class ColorPicker {
  enum ColorType {
    COLOR_UNKNOWN = 0,
    COLOR_HSV = 1,
    COLOR_RGB = 2,
    COLOR_ALPHA = 3,
  };

  vec4_t pickerHSV{};
  vec4_t pickerRGB{};

  vec4_t HSV{};
  vec4_t fullRGB{}, normalizedRGB{};

  vec4_t oldRGB{};

  vec4_t pickerColor{};

  // the cvar that the color picker is currently modifying
  std::string currentCvar;
  std::string currentCvarOldValue;

  bool normalizedRGBSliders;

  static inline void setHSV(vec4_t hsv, float h, float s, float v, float a);
  static inline void setRGB(vec4_t rgb, float r, float g, float b, float a);
  static inline void setRGBNormalized(vec4_t rgb, float r, float g, float b,
                                      float a);

  void updateRGBSliderState(const vec4_t rgb) const;
  static void updateHSVSliderState(const vec4_t hsv);

public:
  ColorPicker();
  ~ColorPicker() = default;

  void drawColorPicker(const rectDef_t *rect);
  void drawPreviewOld(const rectDef_t *rect) const;
  void drawPreviewNew(const rectDef_t *rect);

  // shrinks a rectDef to match the color area
  // color picker rects have a bit of padding on sides to draw the borders,
  // so this just modifies the window rect to match the actual color area
  static void shrinkRectForColorPicker(rectDef_t &rect);

  void cvarToColorPickerState(const std::string &cvar);

  // if reset is true, value is taken from currentCvarOldValue
  void colorPickerStateToCvar(bool reset) const;

  // reset sliders to state at which they were in when the menu was opened
  void resetColorPickerState();

  static const char *getColorSliderString(int handle);
  static void setColorSliderType(itemDef_t *item);
  static float getColorSliderValue(const std::string &colorVar);
  static void setColorSliderValue(const std::string &colorVar, float value);
  void updateSliderState(const itemDef_t *item);

  void colorPickerDragFunc(const itemDef_t *item, float cursorX, float cursorY,
                           int key);

  // toggle RGB sliders between normalized and full RGB
  void toggleRGBSliderValues();
  [[nodiscard]] bool RGBSlidersAreNormalized() const;
};
} // namespace ETJump
