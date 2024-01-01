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

#pragma once

#ifdef min
  #undef min
#endif
#ifdef max
  #undef max
#endif

#include <string>
#include <functional>
#include <map>
#include <vector>

namespace Utilities {
class ResultSetFormatter {
public:
  typedef std::map<std::string, std::string> Row;

  ResultSetFormatter();
  ~ResultSetFormatter();

  /**
   * @param headers Displayed headers. If header does not exist on row,
   * an empty column will be displayed
   * @param rows Data
   * @param pageSize
   * @param page
   * @returns the buffer with the data
   */
  std::string toString(std::vector<std::string> headers, std::vector<Row> rows,
                       int pageSize, int page);

private:
  unsigned totalWidth(std::map<std::string, unsigned> columnWidths);
};
} // namespace Utilities
