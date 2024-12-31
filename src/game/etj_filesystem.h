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

#pragma once

#include <string>
#include <vector>

namespace ETJump {
class FileSystem {
  static constexpr int BIG_DIR_BUFFER = 2 << 20;

public:
  static void copy(const std::string &src, const std::string &dst);
  static void move(const std::string &src, const std::string &dst);
  static bool remove(const std::string &path);
  static bool exists(const std::string &path);
  static bool safeCopy(const std::string &src, const std::string &dst);
  static bool safeMove(const std::string &src, const std::string &dst);
  static std::vector<std::string>

  // if sort is true, the returned list is sorted case-insensitively
  getFileList(const std::string &path, const std::string &ext, bool sort);

  class Path {
    static std::string buildOSPath(const std::string &file);

  public:
    static std::string sanitize(std::string path);
    static std::string sanitizeFile(std::string path);
    static std::string sanitizeFolder(std::string path);

    // returns the full filesystem path to a file
    static std::string getPath(const std::string &file);
  };
};
} // namespace ETJump
