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

#include "etj_file.h"

#ifdef GAMEDLL
  #include "g_local.h"
  #include "etj_string_utilities.h"
#elif CGAMEDLL
  #include "../cgame/cg_local.h"
#endif

ETJump::File::File(std::string path, Mode mode)
    : _path(std::move(path)), _handle(INVALID_FILE_HANDLE), _mode(mode) {
  fsMode_t fsMode;
  switch (_mode) {
    case Mode::Read:
      fsMode = FS_READ;
      break;
    case Mode::Write:
      fsMode = FS_WRITE;
      break;
    case Mode::Append:
      fsMode = FS_APPEND;
      break;
    case Mode::AppendSync:
      fsMode = FS_APPEND_SYNC;
      break;
    default:
      fsMode = FS_READ;
      break;
  }

  _length = trap_FS_FOpenFile(_path.c_str(), &_handle, fsMode);
  if (_mode == Mode::Read && _length == FILE_NOT_FOUND) {
    throw FileNotFoundException(_path);
  }
}

ETJump::File::~File() {
  if (_handle > INVALID_FILE_HANDLE) {
    trap_FS_FCloseFile(_handle);
  }
}

std::vector<char> ETJump::File::read(int bytes) {
  if (_mode != Mode::Read) {
    throw std::logic_error(
        "Cannot read from a file when mode is not Mode::Read.");
  }
  auto readBytes =
      bytes == READ_ALL_BYTES ? _length : (_length >= bytes ? bytes : _length);
  auto buffer = std::vector<char>(readBytes);

  trap_FS_Read(buffer.data(), readBytes, _handle);
  return buffer;
}

void ETJump::File::write(const std::string &data) const {
  write(data.c_str(), static_cast<int>(data.length()));
}

void ETJump::File::write(const std::vector<char> &data) const {
  write(data.data(), static_cast<int>(data.size()));
}

void ETJump::File::write(const char *data, int len) const {
  if (_mode == Mode::Read) {
    throw std::logic_error("Cannot write to a file when mode is Mode::Read.");
  }

#ifdef GAMEDLL
  auto bytesWritten = trap_FS_Write(data, len, _handle);
  if (bytesWritten != len) {
    throw WriteFailedException(ETJump::stringFormat(
        "Write to file %s failed. Wrote %d out of %d bytes.", _path,
        bytesWritten, len));
  }
#else
  trap_FS_Write(data, len, _handle);
#endif
}
