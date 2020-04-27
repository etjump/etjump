/*
 * MIT License
 * 
 * Copyright (c) 2020 ETJump team <haapanen.jussi@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
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
#include <boost/format.hpp>

const char *G_SHA1(const char *str);

ETJump::OperatingSystem::OperatingSystem()
{
}

void ETJump::OperatingSystem::minimize()
{
}

std::string ETJump::OperatingSystem::getHwid()
{
	struct ifreq  ifr;
	struct ifconf ifc;
	char          buf[1024];
	int           success = 0;

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1)
	{                 /* handle error*/
	}
	;

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1)
	{                                           /* handle error */
	}

	struct ifreq              *it = ifc.ifc_req;
	const struct ifreq *const end = it + (ifc.ifc_len / sizeof(struct ifreq));

	for (; it != end; ++it)
	{
		strcpy(ifr.ifr_name, it->ifr_name);
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0)
		{
			if (!(ifr.ifr_flags & IFF_LOOPBACK))   // don't count loopback
			{
				if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0)
				{
					success = 1;
					break;
				}
			}
		}
		else
		{        /* handle error */
		}
	}

	unsigned char mac_address[6];

	if (success)
	{
		memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);
		boost::format fmt("%02X:%02X:%02X:%02X:%02X:%02X");
		fmt % mac_address[0]
			% mac_address[1]
			% mac_address[2]
			% mac_address[3]
			% mac_address[4]
			% mac_address[5];
		return G_SHA1(fmt.str().c_str());
	}
	else
	{
		return "NOHWID";
	}
}

#endif