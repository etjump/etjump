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

#include "etj_file.h"
#include "etj_filesystem.h"

#if defined(GAMEDLL)
  #include "g_local.h"
  #include "etj_string_utilities.h"
#elif defined(CGAMEDLL)
  #include "../cgame/cg_local.h"
  #include "etj_string_utilities.h"
#elif defined(UIDLL)
  #include "../ui/ui_local.h"
#endif

namespace ETJump {
File::File(std::string filename, const Mode mode)
    : file(std::move(filename)), handle(INVALID_FILE_HANDLE), mode(mode) {
  fsMode_t fsMode{};

  switch (mode) {
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
      Com_Error(ERR_FATAL, "%s: bad mode %i for file '%s'\n", __func__,
                static_cast<int>(mode),
                FileSystem::Path::getPath(file).c_str());
  }

  length = trap_FS_FOpenFile(file.c_str(), &handle, fsMode);

  // FS API is the best, isn't it
  if (mode == Mode::Read) {
    if (length <= FILE_NOT_FOUND) {
      throw FileIOException(file);
    }
  } else {
    if (length < FILE_NOT_FOUND) {
      throw FileIOException(file);
    }
  }
}

File::~File() {
  if (handle > INVALID_FILE_HANDLE) {
    trap_FS_FCloseFile(handle);
  }
}

std::vector<char> File::read(const int bytes) const {
  if (mode != Mode::Read) {
    throw FileIOException(
        "Cannot read from a file when mode is not Mode::Read.");
  }

  const int readBytes =
      bytes == READ_ALL_BYTES ? length : std::max(bytes, length);
  auto buffer = std::vector<char>(readBytes);

  trap_FS_Read(buffer.data(), readBytes, handle);
  return buffer;
}

void File::write(const std::string &data) const {
  write(data.c_str(), static_cast<int>(data.length()));
}

void File::write(const std::vector<char> &data) const {
  write(data.data(), static_cast<int>(data.size()));
}

void File::write(const char *data, int len) const {
  if (mode == Mode::Read) {
    throw FileIOException("Cannot write to a file when mode is Mode::Read.");
  }

#ifdef UIDLL
  trap_FS_Write(data, len, handle);
#else
  const auto bytesWritten = trap_FS_Write(data, len, handle);

  if (bytesWritten != len) {
    throw FileIOException(
        stringFormat("Write to file '%s' failed. Wrote %d out of %d bytes.",
                     FileSystem::Path::getPath(file), bytesWritten, len));
  }
#endif
}
} // namespace ETJump
