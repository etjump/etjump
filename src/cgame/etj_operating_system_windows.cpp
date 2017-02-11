#include "etj_operating_system.h"
#ifdef WIN32

#include <Windows.h>
#include "etj_client_authentication.h"
const char *G_SHA1(const char *str);

void ETJump::OperatingSystem::minimize()
{
	HWND wnd = GetForegroundWindow();
	if (wnd)
	{
		ShowWindow(wnd, SW_MINIMIZE);
	}
}

std::string ETJump::OperatingSystem::getHwid()
{
	std::string hardwareId = "";
	std::string rootDrive = "";
	int   systemInfoSum = 0;
	char  vsnc[MAX_PATH] = "\0";
	DWORD vsn;

	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	// Random data from processor
	systemInfoSum = systemInfo.dwProcessorType +
		systemInfo.wProcessorLevel + systemInfo.wProcessorArchitecture;

	char buffer[MAX_PATH]{};
	_itoa(systemInfoSum, buffer, 10);
	hardwareId += buffer;
	// volume serial number
	GetEnvironmentVariable("HOMEDRIVE", buffer, sizeof(buffer));
	rootDrive = std::string(buffer) + "\\";

	if (GetVolumeInformation(rootDrive.c_str(), 0, 0, &vsn, 0, 0, 0, 0) == 0)
	{
		// Failed to get volume info
		rootDrive += "failed";
	}

	_itoa(vsn, buffer, 10);
	hardwareId += buffer;

	return G_SHA1(hardwareId.c_str());
}

#endif