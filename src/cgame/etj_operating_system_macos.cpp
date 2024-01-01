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

#ifdef __APPLE__

  #include "etj_operating_system.h"

  #include <IOKit/IOKitLib.h>

const char *G_SHA1(const char *str);

ETJump::OperatingSystem::OperatingSystem() = default;

void ETJump::OperatingSystem::minimize() {}

std::string ETJump::OperatingSystem::getHwid() {
  char buf[512];
  io_registry_entry_t ioRegistryRoot =
      IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
  auto uuidCf = (CFStringRef)IORegistryEntryCreateCFProperty(
      ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
  IOObjectRelease(ioRegistryRoot);
  CFStringGetCString(uuidCf, buf, sizeof(buf), kCFStringEncodingMacRoman);
  CFRelease(uuidCf);

  return G_SHA1(buf);
}

#endif
