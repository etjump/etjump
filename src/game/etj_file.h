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
#include <sstream>
#include <stdexcept>
#include <utility>

namespace ETJump {
class File {
public:
  class FileIOException : public std::runtime_error {
  public:
    explicit FileIOException(const std::string &what)
        : std::runtime_error(what) {}
  };

  typedef int FileHandle;
  enum class Mode { Read, Write, Append, AppendSync };

  static constexpr int FILE_NOT_FOUND = -1;
  static constexpr int INVALID_FILE_HANDLE = 0;
  static constexpr int READ_ALL_BYTES = -1;

  // throws FileIOException if file handle cannot be opened
  // (file not found, path is not writable etc.)
  explicit File(std::string path, Mode mode = Mode::Read);
  ~File();

  // reads `bytes` bytes from the file.
  // if file length < bytes, reads length bytes
  std::vector<char> read(int bytes = READ_ALL_BYTES) const;

  // writes all data to the file
  // throws FileIOException if operation fails
  void write(const std::string &data) const;
  void write(const std::vector<char> &data) const;
  void write(const char *data, int len) const;

private:
  std::string _path;
  FileHandle _handle;
  int _length;
  Mode _mode;
};
} // namespace ETJump
