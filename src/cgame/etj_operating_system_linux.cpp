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

#ifdef __linux__

  #include "etj_operating_system.h"
  #include <sys/ioctl.h>
  #include <net/if.h>
  #include <netinet/in.h>
  #include <cstring>
  #include <unistd.h>
  #include <fstream>
  #include "../game/etj_string_utilities.h"

const char *G_SHA1(const char *str);

ETJump::OperatingSystem::OperatingSystem() = default;

void ETJump::OperatingSystem::minimize() {}

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

  if (!success) {
    closeSocket(sock);
    return "NOHWID";
  }

  unsigned char mac_address[6];
  memcpy(mac_address, ifr.ifr_hwaddr.sa_data, sizeof(mac_address));
  hwid += stringFormat("%02X:%02X:%02X:%02X:%02X:%02X", mac_address[0],
                       mac_address[1], mac_address[2], mac_address[3],
                       mac_address[4], mac_address[5]);

  // TODO: include this in HWID, when user database has been refactored
  //  to store HWIDs of individual components
  /*
  std::ifstream machineID("/etc/machine-id");

  if (!machineID) {
    return "NOHWID";
  }

  std::string id;
  std::getline(machineID, id);

  if (id.empty()) {
    return "NOHWID";
  }

  hwid += id;
  */

  return G_SHA1(hwid.c_str());
}

#endif
