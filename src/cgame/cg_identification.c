#include "cg_local.h"

// I guess there's no great way of obtaining a secure hwid on linux.
#if defined __linux__

void CG_Minimize_f(void) {
	return;
}

#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>

char *CG_getClientHWID(void)  {
	struct ifreq ifr;
	struct ifconf ifc;
	char buf[1024];
	int success = 0;

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1) { /* handle error*/ };

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { /* handle error */ }

	struct ifreq* it = ifc.ifc_req;
	const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

	for (; it != end; ++it) {
		strcpy(ifr.ifr_name, it->ifr_name);
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
			if (!(ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
				if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
					success = 1;
					break;
				}
			}
		} else { /* handle error */ }
	}

	unsigned char mac_address[6];

	if (success) {
		memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);
		return G_SHA1(va("%02X:%02X:%02X:%02X:%02X:%02X",
															mac_address[0],
															mac_address[1],
															mac_address[2],
															mac_address[3],
															mac_address[4],
															mac_address[5]));
	} else {
		return "NOHWID";
	}
}

#elif defined WIN32
#define Rectangle LCC_Rectangle
#include <Windows.h>
#undef Rectangle

// Zero: this doesn't belong here, but I didn't feel like including <Windows.h> again
// just for this 

void CG_Minimize_f ( void ) 
{
	HWND wnd;
	if(wnd = GetForegroundWindow())
	{
		ShowWindow(wnd, SW_MINIMIZE);
	}
}

// Maybe not the best hardware ID but should do the trick, I hope.
char *CG_getClientHWID(void) 
{
    int systemInfoSum = 0;
    char hwId[MAX_TOKEN_CHARS] = "\0";
    char rootdrive[MAX_PATH] = "\0";
    char vsnc[MAX_PATH] = "\0";
    DWORD vsn;
   
    SYSTEM_INFO systemInfo;
    GetSystemInfo( &systemInfo );

    // Random data from processor
    systemInfoSum = systemInfo.dwProcessorType + systemInfo.wProcessorLevel + systemInfo.wProcessorArchitecture;

    itoa(systemInfoSum, hwId, 10);
	// volume serial number
    GetEnvironmentVariable("HOMEDRIVE", rootdrive, sizeof(rootdrive));
    Q_strcat(rootdrive, sizeof(rootdrive), "\\");

    if(GetVolumeInformation(rootdrive, 0, 0, &vsn, 0, 0, 0, 0) == 0)
    {
        // Failed to get volume info
        Q_strcat(vsnc, sizeof(vsnc), "failed");
    }

    itoa(vsn, vsnc, 10);

    Q_strcat(hwId, sizeof(hwId), vsnc);
    
	return G_SHA1(hwId);
}

#endif

