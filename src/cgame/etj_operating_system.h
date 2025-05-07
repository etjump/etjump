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

#ifdef NEW_AUTH
  #include <vector>

  #ifdef WIN32
    #include <Wbemidl.h>

    #pragma comment(lib, "wbemuuid.lib")
  #endif
#endif

namespace ETJump {
class OperatingSystem {
public:
  OperatingSystem();
  static void minimize();

#ifdef NEW_AUTH
  static constexpr char NOHWID[] = "NOHWID";

  static int getOS();
  static std::vector<std::string> getHwid();

private:
  enum CPUIDRegisters {
    EAX,
    EBX,
    ECX,
    EDX,
    NUM_CPUID_REGISTERS,
  };

  #if defined(WIN32) || defined(__linux__)
  // returns information via CPUID (vendor, family, model, full name, stepping)
  static std::string getCPUInfo();

  // returns the MAC address of the active network adapter
  static std::string getMACAddress();

  // win: returns the OS disk serial number as reported by the WMI
  // linux: returns the OS disk serial number as reported by the kernel
  static std::string getDiskInfo();

    #if defined(WIN32)
  // returns the SID of the current Windows user
  static std::string getCurrentUserSID();

  // returns the system UUID
  static std::string getSystemUUID();

  // returns the motherboard serial number
  static std::string getMBSerial();

  // returns a single property value from a WMI query
  // if the queried WMI class returns multiple objects,
  // only the properties of the first result are read
  // return value is *NOT* hashed!
  static std::string getWMIProperty(const std::wstring &wmiClass,
                                    const std::wstring &propName);

  // sets up WMI connection
  static IWbemServices *getWMIService();
    #endif

    #if defined(__linux__)
  // returns the system ID as reported by /etc/machine-id
  static std::string getMachineID();
    #endif
  #endif

#else
  static std::string getHwid();
#endif
};
} // namespace ETJump
