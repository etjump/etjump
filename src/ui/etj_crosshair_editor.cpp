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

#include "etj_crosshair_editor.h"
#include "etj_local.h"
#include "etj_ui.h"
#include "ui_local.h"

#include "../cgame/etj_color_parser.h"
#include "../game/etj_file.h"
#include "../game/etj_filesystem.h"
#include "../game/etj_string_utilities.h"

#include <algorithm>
#include <cmath>

// set while an edit field has the keyboard. defined at global scope in
// ui_shared.cpp, which has no header for it, so declare it here rather than
// touch the shared header.
extern qboolean g_editingField;

namespace ETJump {
namespace {
// one cvar per editable field for the widgets to bind to, folded back into the
// packed element cvar every frame
constexpr const char *cvarSlot = "etj_ceSlot";
constexpr const char *cvarType = "etj_ceType";
constexpr const char *cvarEnabled = "etj_ceEnabled";
constexpr const char *cvarLength = "etj_ceLength";
constexpr const char *cvarLengthY = "etj_ceLengthY";
constexpr const char *cvarGap = "etj_ceGap";
constexpr const char *cvarGapY = "etj_ceGapY";
constexpr const char *cvarThickness = "etj_ceThickness";
constexpr const char *cvarOffsetX = "etj_ceOffsetX";
constexpr const char *cvarOffsetY = "etj_ceOffsetY";
constexpr const char *cvarRotation = "etj_ceRotation";
constexpr const char *cvarOutline = "etj_ceOutline";
constexpr const char *cvarOutlineThickness = "etj_ceOutlineThickness";
constexpr const char *cvarFill = "etj_ceFill";
constexpr const char *cvarColor = "etj_ceColor";
constexpr const char *cvarAlpha = "etj_ceAlpha";
constexpr const char *cvarColorAlt = "etj_ceColorAlt";
constexpr const char *cvarAlphaAlt = "etj_ceAlphaAlt";
constexpr const char *cvarOutlineColor = "etj_ceOutlineColor";
constexpr const char *cvarOutlineAlpha = "etj_ceOutlineAlpha";

constexpr const char *presetNameCvar = "etj_cePresetName";
constexpr const char *presetExtension = ".cfg";

std::string getString(const char *cvar) {
  char buffer[MAX_CVAR_VALUE_STRING]{};
  trap_Cvar_VariableStringBuffer(cvar, buffer, sizeof(buffer));
  return buffer;
}

float getFloat(const char *cvar) { return trap_Cvar_VariableValue(cvar); }

void setString(const char *cvar, const std::string &value) {
  trap_Cvar_Set(cvar, value.c_str());
}

void setFloat(const char *cvar, const float value) {
  auto str = StringUtils::normalizeNumberString(std::to_string(value));

  if (str.empty()) {
    str = "0";
  }

  trap_Cvar_Set(cvar, str.c_str());
}

void setBool(const char *cvar, const bool value) {
  trap_Cvar_Set(cvar, value ? "1" : "0");
}
} // namespace

std::string CrosshairEditor::slotCvarName(const int slot) {
  return "etj_crosshairElement" + std::to_string(slot);
}

std::string CrosshairEditor::presetDir() {
  // has to live under profiles/, save/ or demos/ - FS_AllowDeletion in the
  // engine refuses to delete anything outside those, so a preset kept in a
  // crosshairs/ folder at the mod root could be written and read but never
  // removed. the profile dir is where per user config belongs anyway.
  auto profile = FileSystem::Path::sanitizeFolder(cl_profile.string);

  if (profile.empty()) {
    profile = "default";
  }

  return "profiles/" + profile + "/crosshairs/";
}

CrosshairDefinition CrosshairEditor::readDefinition() {
  CrosshairDefinition definition;

  for (int i = 0; i < CrosshairLimits::maxElements; i++) {
    auto &element = definition.elements[i];
    element = parseCrosshairElement(getString(slotCvarName(i + 1).c_str()));
    element.resolveColors([](const std::string &colorString, vec4_t &out) {
      ui.colorParser->parseColorString(colorString, out);
    });
  }

  return definition;
}

CrosshairElement CrosshairEditor::elementFromWorkingCvars() {
  CrosshairElement element{};

  element.type = static_cast<CrosshairElementType>(
      static_cast<int>(std::lround(getFloat(cvarType))));
  element.enabled = getFloat(cvarEnabled) != 0.0f;
  element.length = getFloat(cvarLength);
  element.lengthY = getFloat(cvarLengthY);
  element.gap = getFloat(cvarGap);
  element.gapY = getFloat(cvarGapY);
  element.thickness = getFloat(cvarThickness);
  element.offsetX = getFloat(cvarOffsetX);
  element.offsetY = getFloat(cvarOffsetY);
  element.rotation = getFloat(cvarRotation);
  element.outline = getFloat(cvarOutline) != 0.0f;
  element.outlineThickness = getFloat(cvarOutlineThickness);
  element.fill = getFloat(cvarFill) != 0.0f;
  element.alpha = getFloat(cvarAlpha);
  element.alphaAlt = getFloat(cvarAlphaAlt);
  element.outlineAlpha = getFloat(cvarOutlineAlpha);

  // empty color cvar keeps the default instead of turning into garbage, which
  // can happen before the working cvars have been written once
  const auto assignColor = [](const char *cvar, std::string &target) {
    const auto value = getString(cvar);

    if (!value.empty()) {
      target = value;
    }
  };

  assignColor(cvarColor, element.colorString);
  assignColor(cvarColorAlt, element.colorAltString);
  assignColor(cvarOutlineColor, element.outlineColorString);

  element.clampToLimits();

  return element;
}

void CrosshairEditor::elementToWorkingCvars(const CrosshairElement &element) {
  setFloat(cvarType, static_cast<float>(element.type));
  setBool(cvarEnabled, element.enabled);
  setFloat(cvarLength, element.length);
  setFloat(cvarLengthY, element.lengthY);
  setFloat(cvarGap, element.gap);
  setFloat(cvarGapY, element.gapY);
  setFloat(cvarThickness, element.thickness);
  setFloat(cvarOffsetX, element.offsetX);
  setFloat(cvarOffsetY, element.offsetY);
  setFloat(cvarRotation, element.rotation);
  setBool(cvarOutline, element.outline);
  setFloat(cvarOutlineThickness, element.outlineThickness);
  setBool(cvarFill, element.fill);
  setString(cvarColor, element.colorString);
  setFloat(cvarAlpha, element.alpha);
  setString(cvarColorAlt, element.colorAltString);
  setFloat(cvarAlphaAlt, element.alphaAlt);
  setString(cvarOutlineColor, element.outlineColorString);
  setFloat(cvarOutlineAlpha, element.outlineAlpha);
}

void CrosshairEditor::loadSlot(const int slot) {
  const auto clamped = std::clamp(slot, 1, CrosshairLimits::maxElements);
  const auto raw = getString(slotCvarName(clamped).c_str());

  elementToWorkingCvars(parseCrosshairElement(raw));

  activeSlot = clamped;
  lastSlotValue = raw;
  lastWidgetValue = serializeCrosshairElement(elementFromWorkingCvars());
}

void CrosshairEditor::tidyWorkingCvars() {
  // the sliders write raw "%f", so a value box would sit there reading
  // "6.000000". rewrite anything that isn't already in its short form.
  static const std::array<const char *, 13> numeric = {
      cvarLength,   cvarLengthY,   cvarGap,           cvarGapY,
      cvarThickness, cvarOffsetX,  cvarOffsetY,       cvarRotation,
      cvarAlpha,    cvarAlphaAlt,  cvarOutlineAlpha,  cvarOutlineThickness,
      "etj_customCrosshairScale"};

  for (const auto *cvar : numeric) {
    const auto current = getString(cvar);

    if (current.empty()) {
      continue;
    }

    const auto tidy =
        StringUtils::normalizeNumberString(std::to_string(getFloat(cvar)));

    if (!tidy.empty() && tidy != current) {
      setString(cvar, tidy);
    }
  }
}

void CrosshairEditor::update() {
  const auto rawSlot = static_cast<int>(std::lround(getFloat(cvarSlot)));
  const auto slot =
      std::clamp(rawSlot, 1, CrosshairLimits::maxElements);

  // the working cvars get created on demand instead of registered, so on the
  // first open etj_ceSlot reads 0, which the dropdown has no entry for. write
  // the clamped value back so it shows a selection.
  if (rawSlot != slot) {
    setFloat(cvarSlot, static_cast<float>(slot));
  }

  // never while someone is typing into a value box, we'd be rewriting the text
  // out from under them mid keystroke
  if (!g_editingField) {
    tidyWorkingCvars();
  }

  // slot changed, or first frame. the slot wins.
  if (slot != activeSlot) {
    loadSlot(slot);
    return;
  }

  const auto slotValue = getString(slotCvarName(slot).c_str());

  // changed under us, so a preset load or a console set. widgets follow.
  if (slotValue != lastSlotValue) {
    loadSlot(slot);
    return;
  }

  // otherwise someone's been dragging sliders, fold those into the slot
  const auto widgetValue =
      serializeCrosshairElement(elementFromWorkingCvars());

  if (widgetValue != lastWidgetValue) {
    setString(slotCvarName(slot).c_str(), widgetValue);
    lastWidgetValue = widgetValue;
    lastSlotValue = widgetValue;
  }
}

const CrosshairPainter &CrosshairEditor::painter() {
  static const CrosshairPainter painter{
      [](const float x, const float y, const float w, const float h,
         const vec4_t color) { UI_FillRect(x, y, w, h, color); },
      [](const float x0, const float y0, const float x1, const float y1,
         const float thickness, const vec4_t color) {
        // axis aligned lines are one rect, anything else gets stepped, same as
        // cgame's drawLineDDA does
        if (x0 == x1) {
          UI_FillRect(x0 - (thickness * 0.5f), std::min(y0, y1), thickness,
                      std::abs(y1 - y0), color);
          return;
        }

        if (y0 == y1) {
          UI_FillRect(std::min(x0, x1), y0 - (thickness * 0.5f),
                      std::abs(x1 - x0), thickness, color);
          return;
        }

        const auto length =
            std::sqrt(((x1 - x0) * (x1 - x0)) + ((y1 - y0) * (y1 - y0)));

        if (length <= 0.0f) {
          return;
        }

        const auto stepX = (x1 - x0) / length;
        const auto stepY = (y1 - y0) / length;
        const auto steps = static_cast<int>(length);

        auto x = x0;
        auto y = y0;

        for (int i = 0; i < steps; i++) {
          UI_FillRect(x - (thickness * 0.5f), y - (thickness * 0.5f), thickness,
                      thickness, color);
          x += stepX;
          y += stepY;
        }
      }};

  return painter;
}

void CrosshairEditor::drawPreview(rectDef_t *rect) {
  update();

  if (!rect) {
    return;
  }

  const auto centerX = rect->x + (rect->w * 0.5f);
  const auto centerY = rect->y + (rect->h * 0.5f);

  // faint center guides so offsets are readable against the box
  const vec4_t guideColor = {1.0f, 1.0f, 1.0f, 0.12f};
  UI_FillRect(rect->x + 2.0f, centerY, rect->w - 4.0f, 1.0f, guideColor);
  UI_FillRect(centerX, rect->y + 2.0f, 1.0f, rect->h - 4.0f, guideColor);

  const auto definition = readDefinition();

  if (definition.activeElements().empty()) {
    const vec4_t textColor = {0.8f, 0.8f, 0.8f, 0.7f};
    const char *message = "no elements";
    const auto textW =
        static_cast<float>(uiInfo.uiDC.textWidth(message, 0.2f, 0));

    uiInfo.uiDC.drawText(centerX - (textW * 0.5f), centerY + 4.0f, 0.2f,
                         textColor, message, 0, 0, ITEM_TEXTSTYLE_SHADOWED);
    return;
  }

  // draw at the real configured scale where it fits, only shrink when it would
  // spill out of the box
  const auto userScale = std::max(getFloat("etj_customCrosshairScale"), 0.0f);
  const auto extent = crosshairDefinitionExtent(definition) * userScale;
  const auto maxExtent = std::min(rect->w, rect->h) * 0.45f;
  const auto fit =
      (extent > maxExtent && extent > 0.0f) ? maxExtent / extent : 1.0f;

  drawCrosshairDefinition(definition, painter(), centerX, centerY,
                          userScale * fit);
}

void CrosshairEditor::refreshPresets() {
  presets = FileSystem::getFileList(presetDir(), presetExtension, true);

  for (auto &preset : presets) {
    StringUtils::stripExtension(preset);
  }

  if (selectedPreset >= static_cast<int>(presets.size())) {
    selectedPreset = presets.empty() ? 0 : static_cast<int>(presets.size()) - 1;
  }
}

int CrosshairEditor::presetCount() const {
  return static_cast<int>(presets.size());
}

const char *CrosshairEditor::presetName(const int index) const {
  if (index < 0 || index >= static_cast<int>(presets.size())) {
    return "";
  }

  return presets[index].c_str();
}

void CrosshairEditor::selectPreset(const int index) {
  if (index < 0 || index >= static_cast<int>(presets.size())) {
    return;
  }

  selectedPreset = index;
  // drop the name into the field too, so saving over the preset you just
  // clicked doesn't mean retyping it
  setString(presetNameCvar, presets[index]);
}

bool CrosshairEditor::savePreset(const std::string &name) {
  const auto sanitized = FileSystem::Path::sanitizeFile(name);

  if (sanitized.empty()) {
    Com_Printf(S_COLOR_RED "Crosshair editor: invalid preset name.\n");
    return false;
  }

  // one element per line so presets stay hand editable like the other configs
  std::string contents;

  for (int i = 1; i <= CrosshairLimits::maxElements; i++) {
    contents += getString(slotCvarName(i).c_str());
    contents += "\n";
  }

  const auto path = presetDir() + sanitized + presetExtension;

  try {
    const File out(path, File::Mode::Write);
    out.write(contents);
  } catch (const File::FileIOException &e) {
    Com_Printf(S_COLOR_RED "Crosshair editor: could not save '%s': %s\n",
               path.c_str(), e.what());
    return false;
  }

  Com_Printf("Crosshair editor: saved preset '%s'.\n", sanitized.c_str());
  return true;
}

bool CrosshairEditor::loadPreset(const std::string &name) {
  const auto sanitized = FileSystem::Path::sanitizeFile(name);

  if (sanitized.empty()) {
    return false;
  }

  const auto path = presetDir() + sanitized + presetExtension;
  std::string contents;

  try {
    const File in(path);
    contents = in.readString(File::READ_ALL_BYTES, true);
  } catch (const File::FileIOException &e) {
    Com_Printf(S_COLOR_RED "Crosshair editor: could not load '%s': %s\n",
               path.c_str(), e.what());
    return false;
  }

  const auto lines = StringUtils::split(contents, "\n");

  // clear any slot the preset doesn't mention, otherwise loading a 2 element
  // preset over an 8 element one leaves leftovers
  for (int i = 0; i < CrosshairLimits::maxElements; i++) {
    const auto line = static_cast<size_t>(i) < lines.size()
                          ? StringUtils::trim(lines[i])
                          : std::string();

    // re-serialize instead of storing the line as is, that validates the file
    // and tidies up anything hand edited
    setString(slotCvarName(i + 1).c_str(),
              serializeCrosshairElement(parseCrosshairElement(line)));
  }

  // force the widgets to re-read the slot
  activeSlot = 0;

  Com_Printf("Crosshair editor: loaded preset '%s'.\n", sanitized.c_str());
  return true;
}

bool CrosshairEditor::saveCurrentPreset() {
  const auto saved = savePreset(getString(presetNameCvar));

  if (saved) {
    refreshPresets();
  }

  return saved;
}

bool CrosshairEditor::loadSelectedPreset() {
  if (presets.empty()) {
    Com_Printf(S_COLOR_YELLOW "Crosshair editor: no preset selected.\n");
    return false;
  }

  return loadPreset(presets[selectedPreset]);
}

bool CrosshairEditor::deleteSelectedPreset() {
  if (presets.empty()) {
    Com_Printf(S_COLOR_YELLOW "Crosshair editor: no preset selected.\n");
    return false;
  }

  const auto deleted = deletePreset(presets[selectedPreset]);

  if (deleted) {
    refreshPresets();
  }

  return deleted;
}

bool CrosshairEditor::deletePreset(const std::string &name) {
  const auto sanitized = FileSystem::Path::sanitizeFile(name);

  if (sanitized.empty()) {
    return false;
  }

  const auto path = presetDir() + sanitized + presetExtension;

  if (!FileSystem::remove(path)) {
    Com_Printf(S_COLOR_RED "Crosshair editor: could not delete '%s'.\n",
               path.c_str());
    return false;
  }

  Com_Printf("Crosshair editor: deleted preset '%s'.\n", sanitized.c_str());
  return true;
}
} // namespace ETJump
