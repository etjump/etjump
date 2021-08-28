/*
 * MIT License
 * 
 * Copyright (c) 2020 ETJump team <zero@etjump.com>
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

#ifdef WIN32

#include <windows.h>
#include "etj_operating_system.h"
#include "etj_client_authentication.h"

const char *G_SHA1(const char *str);
void trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer, int bufsize);

static void addMinimizeButton()
{
	char buffer[64];
	auto *WindowClassName = "Enemy Territory";
	trap_Cvar_VariableStringBuffer("win_hinstance", buffer, sizeof buffer);
	const auto etHandle = reinterpret_cast<HINSTANCE>(atoll(buffer));
	HWND wnd = nullptr;
	while ((wnd = FindWindowEx(nullptr, wnd, WindowClassName, WindowClassName)))
	{
		const auto hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(wnd, GWLP_HINSTANCE));
		if (etHandle == hInst)
		{
			const auto style = GetWindowLongPtr(wnd, GWL_STYLE);
			SetWindowLongPtr(wnd, GWL_STYLE, style | WS_MINIMIZEBOX);
			break;
		}
	}
}

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

#endif
