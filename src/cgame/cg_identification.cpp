extern "C" {
#include "cg_local.h"
}

#include <string>
#include <fstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>

using std::string;

static std::string user_guid;

std::string GenerateUUID() {
    boost::uuids::random_generator gen;
    boost::uuids::uuid u = gen();

    return boost::lexical_cast<std::string>(u);
    // return boost::uuids::to_string(u);
}

std::string G_SHA1(const std::string& str) {
    char s[MAX_TOKEN_CHARS];
    Q_strncpyz(s, str.c_str(), sizeof(s));
    return std::string(G_SHA1(s));
}

void GenerateGuid() {

    char folder_name[MAX_TOKEN_CHARS];
    std::string file_name;

    trap_Cvar_VariableStringBuffer("fs_game", folder_name, sizeof(folder_name));

    if(strlen(folder_name) == 0) {
        file_name = "etjump/etguid";
    } else {
        file_name = std::string(folder_name) + "/etguid";
    }

    std::string etguid = G_SHA1( GenerateUUID() );

    std::ofstream etguid_file(file_name.c_str());

    if(!etguid_file) {
        // Just use the random guid but don't save it anywhere
        user_guid = etguid;
        CG_Printf("^1Error:^7 failed to save guid to a file\n");
        return;
    }

    // Write the guid to the file
    etguid_file << etguid;
    
    etguid_file.close();

    user_guid = etguid;
}

bool ReadGuid() {

    char folder_name[MAX_TOKEN_CHARS];
    std::string file_name;

    trap_Cvar_VariableStringBuffer("fs_game", folder_name, sizeof(folder_name));

    if(strlen(folder_name) == 0) {
        file_name = "etjump/etguid";
    } else {
        file_name = std::string(folder_name) + "/etguid";
    }

    std::ifstream etguid_file(file_name.c_str());

    if(!etguid_file) {
        // Couldn't find the file, create it
        GenerateGuid();
        return true;
    }

    std::string etguid;
    std::getline(etguid_file, etguid);

    if(etguid.length() != 40) {
        GenerateGuid();
        return true;
    }

    user_guid = etguid;
    return true;
}

void SendGuid() {
   ReadGuid();
   // Hash it again so it's not sent as "plain text"
   trap_SendClientCommand((std::string("etguid " + G_SHA1(user_guid)).c_str()));
}

void AdminLogin() {
    std::string username = cg_username.string;
    std::string password = cg_adminpassword.string;
    
    if(username.length() == 0) {
        return;
    }

    if(password.length() == 0) {
        return;
    }

    // adminlogin username password
    trap_SendClientCommand(std::string("login " + username + " " + 
        G_SHA1("ETJump" + password)).c_str());
}

#if defined __linux__

#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>

void CG_Minimize_f(void) {
    return;
}

/*
 * Sends a hashed MAC address as I didn't find a decent HWID
 * library for linux
 */

char *GetHWID(void) {
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

char *GetHWID(void) {
    int systemInfoSum = 0;
    char hwId[MAX_TOKEN_CHARS] = "\0";
    char rootdrive[MAX_PATH] = "\0";
    char vsnc[MAX_PATH] = "\0";
    DWORD vsn;

    SYSTEM_INFO systemInfo;
    GetSystemInfo( &systemInfo );

    // Random data from processor
    systemInfoSum = systemInfo.dwProcessorType + 
        systemInfo.wProcessorLevel + systemInfo.wProcessorArchitecture;

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

const string NO_HARDWARE_ID = "NOHWID\n";

void SendHWID(void) {
    char *clientHWID = GetHWID();

    if(!clientHWID || !Q_stricmp(clientHWID, "NOHWID")) {
        trap_SendConsoleCommand(NO_HARDWARE_ID.c_str());
    } else {
        trap_SendConsoleCommand(va("HWID %s\n", clientHWID));
    }
}

void UserinfoSendHWID(void) {
    char *clientHWID = GetHWID();

    if(!clientHWID || !Q_stricmp(clientHWID, "NOHWID")) {
        trap_Cvar_Set("hwinfo", "NOHWID");
    } else {
        trap_Cvar_Set("hwinfo", clientHWID);
    }
}

/*
 * Doesn't really belong here, but didn't feel like including Windows.h
 * again.
 */

void CG_Minimize_f ( void ) 
{
    HWND wnd;
    if(wnd = GetForegroundWindow())
    {
        ShowWindow(wnd, SW_MINIMIZE);
    }
}

#endif