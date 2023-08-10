/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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
#include <functional>

#include "../game/q_shared.h"

namespace ETJump {
typedef std::initializer_list<const char *> ShaderStage;
typedef std::initializer_list<ShaderStage> ShaderStages;
std::string composeShader(const char *name, ShaderStage general,
                          ShaderStages stages);
std::string composeShader(const char *name, ShaderStages stages);
void parseColorString(const std::string &colorString, vec4_t &color);

template <typename T, std::size_t N>
constexpr std::size_t nelem(T (&)[N]) {
  return N;
}

#ifdef CGAMEDLL

int setTimeout(std::function<void()> fun, int delay);

bool clearTimeout(int handle);

int setInterval(std::function<void()> fun, int delay);

bool clearInterval(int handle);

int setImmediate(std::function<void()> fun);

bool clearImmediate(int handle);

void executeTimeout(int handle);

// checks if a cfg file with given name exists, omit .cfg extension
bool configFileExists(const std::string &filename);

// executes a cfg file with given name, omit .cfg extension
void execFile(const std::string &filename);

#endif
} // namespace ETJump
