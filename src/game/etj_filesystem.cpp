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

#include <stdexcept>
#include <algorithm>

#include "../game/etj_file.h"
#include "../game/etj_string_utilities.h"
#include "../game/etj_filesystem.h"

#ifdef GAMEDLL
  #include "g_local.h"
#elif CGAMEDLL
  #include "../cgame/cg_local.h"
#elif UIDLL
  #include "../ui/ui_local.h"
#endif

namespace ETJump {
void FileSystem::copy(const std::string &src, const std::string &dst) {
  if (src == dst) {
    return;
  }

  const File srcFile(src, File::Mode::Read);
  const File dstFile(dst, File::Mode::Write);
  dstFile.write(srcFile.read());
}

// FIXME: THIS IS NOT SAFE AND CAN BLOW UP!
//  add safeMove if you actually want to use this somewhere (see safeCopy)
void FileSystem::move(const std::string &src, const std::string &dst) {
  copy(src, dst);
  remove(src);
}

bool FileSystem::remove(const std::string &path) {
#ifdef GAMEDLL
  // hacky fallback because qagame doesn't have trap_FS_Delete
  trap_FS_Rename(path.c_str(), "");
  return true;
#else
  const int success = trap_FS_Delete(path.c_str());
  return success == 1;
#endif
}

bool FileSystem::exists(const std::string &path) {
  const int length = trap_FS_FOpenFile(path.c_str(), nullptr, FS_READ);
  return length != File::FILE_NOT_FOUND;
}

bool FileSystem::safeCopy(const std::string &src, const std::string &dst) {
  try {
    copy(src, dst);
  } catch (const File::FileIOException &e) {
    Com_Printf("%s: failed to copy file '%s' to '%s'\n", __func__, src.c_str(),
               dst.c_str());

    if (*e.what()) {
      Com_Printf("%s\n", e.what());
    }

    return false;
  }

  return true;
}

bool FileSystem::safeMove(const std::string &src, const std::string &dst) {
  return safeCopy(src, dst) && remove(src);
}

std::vector<std::string> FileSystem::getFileList(const std::string &path,
                                                 const std::string &ext,
                                                 const bool sort) {
  const auto fileList = std::make_unique<char[]>(BIG_DIR_BUFFER);
  const int numFiles = trap_FS_GetFileList(path.c_str(), ext.c_str(),
                                           fileList.get(), BIG_DIR_BUFFER);

  std::vector<std::string> files;
  files.reserve(numFiles);

  char *namePtr = fileList.get();

  for (auto i = 0; i < numFiles; ++i) {
    files.emplace_back(namePtr);
    namePtr += strlen(namePtr) + 1;
  }

  if (sort) {
    StringUtil::sortStrings(files, true);
  }

  return files;
}

std::string FileSystem::Path::getPath(const std::string &file) {
  std::string osPath = buildOSPath(file);

  if (osPath.empty()) {
    Com_Printf("^1ERROR: failed to build OS path for file %s.\n", file.c_str());
    return {};
  }

  return osPath;
}

std::string FileSystem::Path::buildOSPath(const std::string &file) {
  const auto cvarToString = [&](const char *cvar) {
    char buf[MAX_CVAR_VALUE_STRING];
    trap_Cvar_VariableStringBuffer(cvar, buf, sizeof(buf));
    return std::string(buf);
  };

  const std::string game = cvarToString("fs_game");
  const std::string home = cvarToString("fs_homepath");
  std::string path = home;

  if (!path.empty() && path.back() != '/') {
    path += '/';
  }

  path += game;

  if (!path.empty() && path.back() != '/') {
    path += '/';
  }

  path += file;
  StringUtil::replaceAll(path, "\\", "/");
  return path;
}

std::string FileSystem::Path::sanitize(const std::string &path) {
  return sanitizeFile(path);
}

std::string FileSystem::Path::sanitizeFile(std::string path) {
  static constexpr char illegalChars[] = "\\/:*?\"<>.|";
  for (char ch : illegalChars) {
    std::replace(path.begin(), path.end(), ch, '_');
  }
  return path;
}

std::string FileSystem::Path::sanitizeFolder(std::string path) {
  static constexpr char illegalChars[] = "\\:*?\"<>.|";
  for (char ch : illegalChars) {
    std::replace(path.begin(), path.end(), ch, '_');
  }
  return path;
}
} // namespace ETJump
