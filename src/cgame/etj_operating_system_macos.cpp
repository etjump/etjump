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

#ifdef __APPLE__

  #include "etj_operating_system.h"
  #include "../game/etj_crypto.h"

  #include <IOKit/IOKitLib.h>

  #ifdef NEW_AUTH
    #include "../game/etj_shared.h"
  #endif

ETJump::OperatingSystem::OperatingSystem() = default;

void ETJump::OperatingSystem::minimize() {}

  #ifdef NEW_AUTH

// TODO: improve this? this is VERY basic
std::vector<std::string> ETJump::OperatingSystem::getHwid() {
  char buf[512]{};
  io_registry_entry_t ioRegistryRoot =
      IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
  auto uuidCf = (CFStringRef)IORegistryEntryCreateCFProperty(
      ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
  IOObjectRelease(ioRegistryRoot);
  CFStringGetCString(uuidCf, buf, sizeof(buf), kCFStringEncodingMacRoman);
  CFRelease(uuidCf);

  std::vector<std::string> hwid{};

  // not sure if this can happen
  if (buf[0] == '\0') {
    hwid.emplace_back(NOHWID);
    return hwid;
  }

  hwid.emplace_back(Crypto::sha1(buf));
  return hwid;
}

int ETJump::OperatingSystem::getOS() {
    #if defined(__arm64__)
  return Constants::OS_MACOS_AARCH64;
    #elif defined(__x86_64__)
  return Constants::OS_MACOS_X86_64;
    #else
  return Constants::OS_DEFAULT;
    #endif
}

  #else

std::string ETJump::OperatingSystem::getHwid() {
  char buf[512];
  io_registry_entry_t ioRegistryRoot =
      IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
  auto uuidCf = (CFStringRef)IORegistryEntryCreateCFProperty(
      ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
  IOObjectRelease(ioRegistryRoot);
  CFStringGetCString(uuidCf, buf, sizeof(buf), kCFStringEncodingMacRoman);
  CFRelease(uuidCf);

  return Crypto::sha1(buf);
}

  #endif
#endif
