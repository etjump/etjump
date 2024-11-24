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

#ifdef WIN32

  #include <windows.h>
  #include <sddl.h>
  #include "etj_operating_system.h"
  #include "etj_client_authentication.h"

const char *G_SHA1(const char *str);
void trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer,
                                    int bufsize);

static void addMinimizeButton() {
  char buffer[64];
  auto *WindowClassName = "Enemy Territory";
  trap_Cvar_VariableStringBuffer("win_hinstance", buffer, sizeof buffer);
  const auto etHandle = reinterpret_cast<HINSTANCE>(atoll(buffer));
  HWND wnd = nullptr;
  while ((wnd = FindWindowEx(nullptr, wnd, WindowClassName, WindowClassName))) {
    const auto hInst =
        reinterpret_cast<HINSTANCE>(GetWindowLongPtr(wnd, GWLP_HINSTANCE));
    if (etHandle == hInst) {
      const auto style = GetWindowLongPtr(wnd, GWL_STYLE);
      SetWindowLongPtr(wnd, GWL_STYLE, style | WS_MINIMIZEBOX);
      break;
    }
  }
}

ETJump::OperatingSystem::OperatingSystem() { addMinimizeButton(); }

void ETJump::OperatingSystem::minimize() {
  HWND wnd = GetForegroundWindow();
  if (wnd) {
    ShowWindow(wnd, SW_MINIMIZE);
  }
}

std::string ETJump::OperatingSystem::getCurrentUserSID() {
  HANDLE hToken = nullptr;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
    return "";
  }

  DWORD dwBufferSize = 0;
  GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwBufferSize);
  if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
    CloseHandle(hToken);
    return "";
  }

  const auto pTokenUser =
      static_cast<PTOKEN_USER>(LocalAlloc(LPTR, dwBufferSize));
  if (!pTokenUser) {
    CloseHandle(hToken);
    return "";
  }

  if (!GetTokenInformation(hToken, TokenUser, pTokenUser, dwBufferSize,
                           &dwBufferSize)) {
    LocalFree(pTokenUser);
    CloseHandle(hToken);
    return "";
  }

  LPWSTR stringSID = nullptr;
  if (!ConvertSidToStringSidW(pTokenUser->User.Sid, &stringSID)) {
    LocalFree(pTokenUser);
    CloseHandle(hToken);
    return "";
  }

  // convert from wide string to ASCII string
  const int size = WideCharToMultiByte(CP_UTF8, 0, stringSID, -1, nullptr, 0,
                                       nullptr, nullptr);
  // -1 because we don't need the null terminator in std::string
  std::string result(size - 1, 0);
  WideCharToMultiByte(CP_UTF8, 0, stringSID, -1, &result[0], size, nullptr,
                      nullptr);

  LocalFree(stringSID);
  LocalFree(pTokenUser);
  CloseHandle(hToken);

  return result;
}

std::string ETJump::OperatingSystem::getHwid() {
  std::string hardwareId;
  std::string rootDrive;
  DWORD vsn = 0;

  // TODO: include once user database is refactored
  //  to store individual components of HWID
  /*
  // Get user SID
  const std::string userSid = getCurrentUserSID();
  hardwareId += userSid;
  */

  SYSTEM_INFO systemInfo;
  GetSystemInfo(&systemInfo);

  // Random data from processor
  const unsigned int systemInfoSum = systemInfo.dwProcessorType +
                                     systemInfo.wProcessorLevel +
                                     systemInfo.wProcessorArchitecture;

  char buffer[MAX_PATH]{};
  _ultoa(systemInfoSum, buffer, 10);
  hardwareId += buffer;

  // volume serial number
  GetEnvironmentVariable("HOMEDRIVE", buffer, sizeof(buffer));
  rootDrive = std::string(buffer) + "\\";

  if (GetVolumeInformation(rootDrive.c_str(), nullptr, 0, &vsn, nullptr,
                           nullptr, nullptr, 0) == 0) {
    // Failed to get volume info
    rootDrive += "failed";
  }

  // TODO: include once user database is refactored
  //  to store individual components of HWID
  // hardwareId += rootDrive;

  _ultoa(vsn, buffer, 10);
  hardwareId += buffer;

  return G_SHA1(hardwareId.c_str());
}

#endif
