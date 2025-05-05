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

#include <fstream>
#include "etj_json_utilities.h"
#include "etj_filesystem.h"
#include "etj_file.h"

namespace ETJump {
bool JsonUtils::writeFile(const std::string &file, const Json::Value &root,
                          std::string *errors) {
  Json::StyledWriter writer;
  const std::string &output = writer.write(root);

  if (file.empty()) {
    if (errors) {
      *errors = "Failed to write JSON file: empty filename";
    }

    return false;
  }

  const File fOut(file, File::Mode::Write);

  try {
    fOut.write(output);
    return true;
  } catch (const File::WriteFailedException &e) {
    if (errors) {
      *errors = stringFormat("Failed to write JSON file: %s", e.what());
    }

    return false;
  }
}

bool JsonUtils::readFile(const std::string &file, Json::Value &root,
                         std::string *errors) {
  std::ifstream fIn(FileSystem::Path::getPath(file));

  if (!fIn) {
    fIn.close();

    if (errors) {
      *errors = stringFormat(
          "Failed to read JSON file: unable to open file '%s' for reading",
          file);
    }

    return false;
  }

  Json::CharReaderBuilder readerBuilder;
  readerBuilder["strictRoot"] = true;
  std::string err;

  if (!parseFromStream(readerBuilder, fIn, &root, &err)) {
    if (errors) {
      *errors = stringFormat("Failed to parse JSON file '%s':\n%s", file, err);
    }

    return false;
  }

  fIn.close();
  return true;
}
} // namespace ETJump
