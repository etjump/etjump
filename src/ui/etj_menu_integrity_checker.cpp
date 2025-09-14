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

#include "etj_menu_integrity_checker.h"
#include "ui_local.h"

#include "../game/etj_string_utilities.h"
#include "../game/etj_filesystem.h"
#include "../game/etj_file.h"
#include "../game/etj_crypto.h"

#if !defined(MENU_HASH_LIST)
  #define MENU_HASH_LIST "N/A"
#endif

namespace ETJump {
inline constexpr char INTEGRITY_VIOLATION_TEXT_LINE1[] =
    "^1MENU INTEGRITY VIOLATION DETECTED";
inline constexpr char INTEGRITY_VIOLATION_TEXT_LINE2[] =
    "^1CHECK CONSOLE FOR MORE INFORMATION";

bool MenuIntegrityChecker::checkIntegrity() {
  if (!Q_stricmp(MENU_HASH_LIST, "N/A")) {
    uiInfo.uiDC.Print(
        S_COLOR_YELLOW
        "WARNING: unable to check menu integrity - hashes not generated!\n");
    return false;
  }

  const auto hashList = generateFileHashList();

  if (hashList.empty()) {
    return false;
  }

  for (const auto &[file, hash] : hashList) {
    try {
      File f("ui/" + file);

      const auto contents = f.read();
      const auto currentHash =
          Crypto::sha1(std::string(contents.cbegin(), contents.cend()));

      if (!StringUtil::iEqual(currentHash, hash)) {
        Com_DPrintf("Integrity check failed on file '%s' - invalid hash!\n",
                    file.c_str());
        return false;
      }
    } catch (const File::FileIOException &) {
      Com_DPrintf("Integrity check failed on file '%s' - file not found!\n",
                  file.c_str());
      return false;
    }
  }

  return true;
}

std::vector<std::pair<std::string, std::string>>
MenuIntegrityChecker::generateFileHashList() {
  std::vector<std::pair<std::string, std::string>> list;

  const auto fileHashKvp = StringUtil::split(MENU_HASH_LIST, ",");

  for (const auto &kvp : fileHashKvp) {
    const size_t pos = kvp.find(':');

    if (pos == std::string::npos) {
      uiInfo.uiDC.Print(
          S_COLOR_YELLOW
          "WARNING: unable to check menu integrity - malformed hash string!\n");
      return {};
    }

    list.emplace_back(kvp.substr(0, pos), kvp.substr(pos + 1));
  }

  return list;
}

void MenuIntegrityChecker::printIntegrityViolation() {
  std::string msg;
  msg += "\n^1***********************************************";
  msg += "\n^1*                                             *";
  msg += "\n^1*     !!! MENU INTEGRITY CHECK FAILED !!!     *";
  msg += "\n^1*                                             *";
  msg += "\n^1***********************************************\n\n";

  msg += "You are running menu files not officially supported by ETJump. This "
         "might break the user interface!\n\n";

  uiInfo.uiDC.Print(msg.c_str());

  const auto badPaks = getBadPaks();

  if (badPaks.empty()) {
    return;
  }

  msg.clear();
  msg = "^1The following paks contain potentially unwanted files:\n\n";

  for (const auto &pak : badPaks) {
    msg += "^1- " + uiInfo.fsGame + "/" + pak + "\n";
  }

  msg += "\n^1Consider removing these files in order to restore menu integrity "
         "and remove this warning.\n\n";

  uiInfo.uiDC.Print(msg.c_str());
}

std::vector<std::string> MenuIntegrityChecker::getBadPaks() {
  // see if we can find the offending packs
  const auto files =
      ETJump::FileSystem::getFileList("../" + uiInfo.fsGame, ".pk3", true);
  const auto pk3name =
      std::string(GAME_NAME) + "-" + std::string(GAME_VERSION) + ".pk3";
  const auto it = std::find(files.cbegin(), files.cend(), pk3name);

  if (it == files.end()) {
    return {};
  }

  std::vector<std::string> badPaks;

  // anything alphabetically after the etjump pk3 might be an offending pak file
  for (auto after = std::next(it); after != files.cend(); ++after) {
    badPaks.emplace_back(after->c_str());
  }

  return badPaks;
}

void MenuIntegrityChecker::printIntegrityWatermark() {
  // courbd 30, see global.menu
  fontInfo_t *font = &uiInfo.uiDC.Assets.fonts[3];

  // shouldn't happen, UNLESS SOMEONE MODIFIES THE MENUS...
  if (!font || font->name[0] == '\0') {
    font = &uiInfo.uiDC.Assets.fonts[uiInfo.activeFont];
  }

  constexpr float size1 = 0.2f;
  constexpr float size2 = 0.16f;

  const auto w1 = static_cast<float>(
      uiInfo.uiDC.textWidthExt(INTEGRITY_VIOLATION_TEXT_LINE1, size1, 0, font));
  const auto w2 = static_cast<float>(
      uiInfo.uiDC.textWidthExt(INTEGRITY_VIOLATION_TEXT_LINE2, size2, 0, font));

  constexpr vec4_t color = {1, 1, 1, 0.75f};

  uiInfo.uiDC.drawTextExt(uiInfo.uiDC.screenWidth / 2 - (w1 * 0.5f),
                          uiInfo.uiDC.screenHeight - 20, size1, size1, color,
                          INTEGRITY_VIOLATION_TEXT_LINE1, 0, 0,
                          ITEM_TEXTSTYLE_SHADOWED, font);

  uiInfo.uiDC.drawTextExt(uiInfo.uiDC.screenWidth / 2 - (w2 * 0.5f),
                          uiInfo.uiDC.screenHeight - 12, size2, size2, color,
                          INTEGRITY_VIOLATION_TEXT_LINE2, 0, 0,
                          ITEM_TEXTSTYLE_SHADOWED, font);
}
} // namespace ETJump
