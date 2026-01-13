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

#ifdef __linux__

  #include "etj_operating_system.h"
  #include <sys/ioctl.h>
  #include <net/if.h>
  #include <netinet/in.h>
  #include <cstring>
  #include <unistd.h>
  #include <fstream>
  #include "../game/etj_string_utilities.h"
  #include "../game/etj_crypto.h"

  #ifdef NEW_AUTH
    #include <array>
    #include <filesystem>
    #include <cpuid.h>

    #include "../game/etj_shared.h"
  #endif

ETJump::OperatingSystem::OperatingSystem() = default;

void ETJump::OperatingSystem::minimize() {}

  #ifdef NEW_AUTH
std::vector<std::string> ETJump::OperatingSystem::getHwid() {
  std::vector<std::string> hwid{};

  hwid.emplace_back(getMACAddress());
  hwid.emplace_back(getCPUInfo());
  hwid.emplace_back(getDiskInfo());
  hwid.emplace_back(getMachineID());

  assert(hwid.size() == Constants::Authentication::HWID_SIZE_LINUX);

  return hwid;
}

std::string ETJump::OperatingSystem::getMACAddress() {
  ifreq ifr{};
  ifconf ifc{};
  char buf[1024]{};
  bool success = false;

  const auto closeSocket = [](const int sock) {
    if (sock >= 0) {
      close(sock);
    }
  };

  const int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sock == -1) {
    closeSocket(sock);
    return NOHWID;
  }

  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
    closeSocket(sock);
    return NOHWID;
  }

  const ifreq *it = ifc.ifc_req;

  for (const ifreq *const end = it + ifc.ifc_len / sizeof(struct ifreq);
       it != end; ++it) {
    strcpy(ifr.ifr_name, it->ifr_name);

    if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
      // don't count loopback
      if (!(ifr.ifr_flags & IFF_LOOPBACK)) {
        if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
          success = true;
          break;
        }
      }
    } else {
      closeSocket(sock);
      return NOHWID;
    }
  }

  closeSocket(sock);

  if (!success) {
    return NOHWID;
  }

  unsigned char mac_address[6];
  memcpy(mac_address, ifr.ifr_hwaddr.sa_data, sizeof(mac_address));

  // while things like 'ip addr' use colons for separating the components,
  // windows uses dashes, so let's make this consistent
  // so that same NIC will produce the same hash on both
  const std::string result = stringFormat(
      "%02X-%02X-%02X-%02X-%02X-%02X", mac_address[0], mac_address[1],
      mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
  return Crypto::sha2(result);
}

std::string ETJump::OperatingSystem::getCPUInfo() {
  std::array<uint32_t, NUM_CPUID_REGISTERS> cpuInfo{};

  // find the CPUID level and short vendor string
  if (__get_cpuid(0x0, &cpuInfo[EAX], &cpuInfo[EBX], &cpuInfo[ECX],
                  &cpuInfo[EDX]) == 0) {
    return NOHWID;
  }

  // note: vendor string register order is EBX -> EDX -> ECX
  const std::string vendor =
      std::string(reinterpret_cast<const char *>(&cpuInfo[EBX]), 4) +
      std::string(reinterpret_cast<const char *>(&cpuInfo[EDX]), 4) +
      std::string(reinterpret_cast<const char *>(&cpuInfo[ECX]), 4);

  // get the CPU info and feature bits
  __get_cpuid(0x1, &cpuInfo[EAX], &cpuInfo[EBX], &cpuInfo[ECX], &cpuInfo[EDX]);

  // stepping (bits 0-3)
  const uint8_t stepping = cpuInfo[EAX] & 0xf;

  // basic cpu family ID (bits 8-11)
  uint8_t cpuFamily = cpuInfo[EAX] >> 8 & 0xf;

  // extended family ID if basic family ID is 0x0f
  // (bits 20–27) + basic family ID (bits 8–11)
  if (cpuFamily == 0x0f) {
    cpuFamily = (cpuInfo[EAX] >> 20 & 0xff) + cpuFamily;
  }

  // cpu model ID (bits 4-7)
  uint8_t model = cpuInfo[EAX] >> 4 & 0xf;

  // extended model ID if family ID is 0x06 or 0x0f
  // (bits 16–19) << 4 | basic model ID (bits 4–7)
  if (cpuFamily == 0x06 || cpuFamily == 0x0f) {
    model = (cpuInfo[EAX] >> 16 & 0xf) << 4 | model;
  }

  // anything in the past 3 decades or so should have extended CPUID level
  // high enough to display the full model name as a string
  std::string vendorExt;

  // find the highest extended CPUID level
  if (!vendor.empty() && __get_cpuid_max(0x80000000, nullptr) >= 0x80000004U) {
    for (uint32_t i = 0x80000002; i <= 0x80000004U; i++) {
      __get_cpuid(i, &cpuInfo[EAX], &cpuInfo[EBX], &cpuInfo[ECX],
                  &cpuInfo[EDX]);

      for (const auto &reg : cpuInfo) {
        vendorExt.append(reinterpret_cast<const char *>(&reg), 4);
      }
    }
  }

  if (vendor.empty() || vendorExt.empty()) {
    return NOHWID;
  }

  // it's very likely that vendorExt contains trailing null bytes
  while (vendorExt.back() == '\0') {
    vendorExt.pop_back();
  }

  const std::string cpuID = stringFormat("%s %u %u %s %u", vendor, cpuFamily,
                                         model, vendorExt, stepping);
  return Crypto::sha2(cpuID);
}

std::string ETJump::OperatingSystem::getMachineID() {
  std::ifstream machineID("/etc/machine-id");

  if (!machineID) {
    return NOHWID;
  }

  std::string id;
  std::getline(machineID, id);

  if (id.empty()) {
    return NOHWID;
  }

  return Crypto::sha2(id);
}

std::string ETJump::OperatingSystem::getDiskInfo() {
  // to avoid this being easily changed by just mounting new partitions,
  // we only grab the root partition block device id, since that's
  // the most likely to stay consistent at all the times
  std::ifstream mounts("/proc/mounts");

  if (!mounts) {
    return NOHWID;
  }

  std::string line;
  std::string mountPath;

  // the format for '/proc/mounts' is
  // <device> <dir> <type> <options> <dump frequency> <fs check order>
  // everything is space delimited, so the line with root partition
  // will have ' / ' after the mount path
  while (std::getline(mounts, line)) {
    if (line.find(" / ") != std::string::npos) {
      std::istringstream iss(line);
      iss >> mountPath;
      break;
    }
  }

  if (mountPath.empty()) {
    return NOHWID;
  }

  // we now have the mount path for root,
  // so we can figure out the actual physical device
  std::filesystem::path physicalDevice{};
  std::filesystem::path partitionName = mountPath;
  std::string partition = partitionName.filename();

  try {
    std::filesystem::path physicalDevicePath = "/sys/class/block/" + partition;
    physicalDevice =
        std::filesystem::read_symlink(physicalDevicePath).lexically_normal();
  } catch (const std::bad_alloc &) {
    return NOHWID;
  } catch (const std::filesystem::filesystem_error &) {
    return NOHWID;
  }

  // real path looks something like this (e.g. for NVMe drives)
  // '../../../nvme0n1/nvme0n1p3' -> we only want the parent dir
  const std::string deviceName = physicalDevice.parent_path().filename();

  // finally, we can get the disk serial
  std::ifstream device("/sys/block/" + deviceName + "/device/serial");

  if (!device) {
    return NOHWID;
  }

  std::string diskID;
  std::getline(device, diskID);

  if (diskID.empty()) {
    return NOHWID;
  }

  diskID = trim(diskID);
  return Crypto::sha2(diskID);
}

int ETJump::OperatingSystem::getOS() {
    #if defined(__i386__)
  return Constants::OS_LINUX_X86;
    #elif defined(__x86_64__)
  return Constants::OS_LINUX_X86_64;
    #else
  return Constants::OS_DEFAULT;
    #endif
}

  #else
std::string ETJump::OperatingSystem::getHwid() {
  ifreq ifr{};
  ifconf ifc{};
  char buf[1024];
  bool success = false;
  std::string hwid;

  const auto closeSocket = [](const int sock) {
    if (sock >= 0) {
      close(sock);
    }
  };

  const int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sock == -1) {
    closeSocket(sock);
    return "NOHWID";
  }

  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
    closeSocket(sock);
    return "NOHWID";
  }

  const ifreq *it = ifc.ifc_req;

  for (const ifreq *const end = it + ifc.ifc_len / sizeof(struct ifreq);
       it != end; ++it) {
    strcpy(ifr.ifr_name, it->ifr_name);

    if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
      // don't count loopback
      if (!(ifr.ifr_flags & IFF_LOOPBACK)) {
        if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
          success = true;
          break;
        }
      }
    } else {
      closeSocket(sock);
      return "NOHWID";
    }
  }

  closeSocket(sock);

  if (!success) {
    return "NOHWID";
  }

  unsigned char mac_address[6];
  memcpy(mac_address, ifr.ifr_hwaddr.sa_data, sizeof(mac_address));
  hwid += stringFormat("%02X:%02X:%02X:%02X:%02X:%02X", mac_address[0],
                       mac_address[1], mac_address[2], mac_address[3],
                       mac_address[4], mac_address[5]);

  return Crypto::sha1(hwid);
}
  #endif
#endif
