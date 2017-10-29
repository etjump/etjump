#include "etj_operating_system.h"
#ifdef WIN32

#include <Windows.h>
#include "etj_client_authentication.h"
const char *G_SHA1(const char *str);
void trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer, int bufsize);

ETJump::OperatingSystem::OperatingSystem()
{
	addMinimizeButton();
}

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

void ETJump::OperatingSystem::addMinimizeButton()
{
	char buffer[64];
	auto *WindowClassName = "Enemy Territory";
	trap_Cvar_VariableStringBuffer("win_hinstance", buffer, sizeof buffer);
	const auto etHandle = reinterpret_cast<HINSTANCE>(atol(buffer));
	HWND wnd = nullptr;
	while ((wnd = FindWindowEx(nullptr, wnd, WindowClassName, WindowClassName)))
	{
		const auto hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(wnd, GWL_HINSTANCE));
		if (etHandle == hInst)
		{
			const auto style = GetWindowLongPtr(wnd, GWL_STYLE);
			SetWindowLongPtr(wnd, GWL_STYLE, style | WS_MINIMIZEBOX);
			break;
		}
	}
}

#endif