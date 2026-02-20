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

#ifdef WIN32

  #include <windows.h>
  #include <sddl.h>
  #include "etj_operating_system.h"
  #include "etj_client_authentication.h"
  #include "../game/etj_crypto.h"

  #include <iostream>

  #ifdef NEW_AUTH
    #include <array>
    #include <iphlpapi.h>
    #pragma comment(lib, "IPHLPAPI.lib")

    #define _WIN32_DCOM
    #include <comdef.h>

    #include "../game/etj_shared.h"
    #include "../game/etj_string_utilities.h"
  #endif

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

  #ifdef NEW_AUTH
std::vector<std::string> ETJump::OperatingSystem::getHwid() {
  std::vector<std::string> hwid{};

  hwid.emplace_back(getMACAddress());
  hwid.emplace_back(getCPUInfo());
  hwid.emplace_back(getDiskInfo());
  hwid.emplace_back(getCurrentUserSID());
  hwid.emplace_back(getSystemUUID());
  hwid.emplace_back(getMBSerial());

  assert(hwid.size() == Constants::Authentication::HWID_SIZE_WIN);

  return hwid;
}

// https://learn.microsoft.com/en-us/windows/win32/api/iptypes/ns-iptypes-ip_adapter_info
std::string ETJump::OperatingSystem::getMACAddress() {
  PIP_ADAPTER_INFO pAdapter = nullptr;
  DWORD dwRetVal = 0;
  ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

  auto pAdapterInfo =
      static_cast<IP_ADAPTER_INFO *>(malloc(sizeof(IP_ADAPTER_INFO)));

  if (pAdapterInfo == nullptr) {
    return NOHWID_MAC_ADDR;
  }

  // Make an initial call to GetAdaptersInfo to get
  // the necessary size into the ulOutBufLen variable
  if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
    free(pAdapterInfo);

    pAdapterInfo = static_cast<IP_ADAPTER_INFO *>(malloc(ulOutBufLen));
    if (pAdapterInfo == nullptr) {
      return NOHWID_MAC_ADDR;
    }
  }

  dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);

  if (dwRetVal != NO_ERROR) {
    free(pAdapterInfo);
    return NOHWID_MAC_ADDR;
  }

  pAdapter = pAdapterInfo;
  std::string macAddress;

  while (pAdapter) {
    // skip invalid adapters to try to ensure that this is the active adapter
    // TODO: this might filter out too aggressively?
    if (pAdapter->Type == MIB_IF_TYPE_LOOPBACK ||
        pAdapter->AddressLength == 0 || pAdapter->Address[0] == 0x00) {
      pAdapter = pAdapter->Next;
      continue;
    }

    for (UINT i = 0; i < pAdapter->AddressLength; i++) {
      if (i == pAdapter->AddressLength - 1) {
        macAddress +=
            stringFormat("%.2X", static_cast<int>(pAdapter->Address[i]));
      } else {
        macAddress +=
            stringFormat("%.2X-", static_cast<int>(pAdapter->Address[i]));
      }
    }

    break;
  }

  free(pAdapterInfo);

  // we didn't find valid adapters somehow
  if (macAddress.empty()) {
    return NOHWID;
  }

  return Crypto::sha2(macAddress);
}

std::string ETJump::OperatingSystem::getCPUInfo() {
  std::array<int, NUM_CPUID_REGISTERS> cpuInfo{};

  // find the CPUID level and short vendor string
  __cpuid(cpuInfo.data(), 0x0);

  // note: vendor string register order is EBX -> EDX -> ECX
  const std::string vendor =
      std::string(reinterpret_cast<const char *>(&cpuInfo[EBX]), 4) +
      std::string(reinterpret_cast<const char *>(&cpuInfo[EDX]), 4) +
      std::string(reinterpret_cast<const char *>(&cpuInfo[ECX]), 4);

  // get the CPU info and feature bits
  __cpuid(cpuInfo.data(), 0x1);

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
  __cpuid(cpuInfo.data(), 0x80000000);
  const int cpuidExt = cpuInfo[EAX];

  if (!vendor.empty() && cpuidExt >= 0x80000004) {
    for (int i = 0x80000002; i <= 0x80000004; i++) {
      __cpuid(cpuInfo.data(), i);

      for (const auto &reg : cpuInfo) {
        vendorExt.append(reinterpret_cast<const char *>(&reg), 4);
      }
    }
  }

  if (vendor.empty() || vendorExt.empty()) {
    return NOHWID_CPUID;
  }

  // it's very likely that vendorExt contains trailing null bytes
  while (vendorExt.back() == '\0') {
    vendorExt.pop_back();
  }

  const std::string cpuID = stringFormat("%s %u %u %s %u", vendor, cpuFamily,
                                         model, vendorExt, stepping);
  return Crypto::sha2(cpuID);
}

// largely based on Microsoft's documentation examples
// https://learn.microsoft.com/en-us/windows/win32/wmisdk/example--getting-wmi-data-from-the-local-computer
std::string ETJump::OperatingSystem::getDiskInfo() {
  HRESULT hr;
  IWbemServices *pServices = getWMIService();

  if (!pServices) {
    return NOHWID_DISK;
  }

  // figure out where Windows is installed
  std::array<char, MAX_PATH> windowsDir{};
  GetWindowsDirectory(windowsDir.data(), MAX_PATH);
  const auto driveLetter =
      std::wstring(windowsDir.begin(), windowsDir.end()).substr(0, 2);

  IEnumWbemClassObject *pEnum = nullptr;
  hr = pServices->ExecQuery(
      _bstr_t(L"WQL"), _bstr_t(L"SELECT * FROM Win32_LogicalDiskToPartition"),
      WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnum);

  if (FAILED(hr)) {
    pServices->Release();
    return NOHWID;
  }

  // find matching partition for the drive letter
  IWbemClassObject *pClsObj = nullptr;
  ULONG ret = 0;
  std::wstring partitionDeviceID;

  while (pEnum->Next(WBEM_INFINITE, 1, &pClsObj, &ret) == S_OK) {
    VARIANT vtAntecedent, vtDependent;
    pClsObj->Get(L"Antecedent", 0, &vtAntecedent, nullptr, nullptr);
    pClsObj->Get(L"Dependent", 0, &vtDependent, nullptr, nullptr);

    const std::wstring dependent = vtDependent.bstrVal;

    if (dependent.find(driveLetter) != std::wstring::npos) {
      std::wstring antecedent = vtAntecedent.bstrVal;
      size_t start = antecedent.find(L"DeviceID=\"");

      if (start != std::wstring::npos) {
        start += wcslen(L"DeviceID=\"");
        const size_t end = antecedent.find(L"\"", start);

        if (end != std::wstring::npos) {
          partitionDeviceID = antecedent.substr(start, end - start);
        }
      }

      VariantClear(&vtAntecedent);
      VariantClear(&vtDependent);
      pClsObj->Release();
      break;
    }

    VariantClear(&vtAntecedent);
    VariantClear(&vtDependent);
    pClsObj->Release();
  }

  pEnum->Release();

  if (partitionDeviceID.empty()) {
    pServices->Release();
    return NOHWID_DISK;
  }

  // find the physical device which has our partition
  hr = pServices->ExecQuery(
      _bstr_t(L"WQL"), _bstr_t(L"SELECT * FROM Win32_DiskDriveToDiskPartition"),
      WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnum);

  if (FAILED(hr)) {
    pServices->Release();
    return NOHWID_DISK;
  }

  std::wstring diskDeviceID;

  while (pEnum->Next(WBEM_INFINITE, 1, &pClsObj, &ret) == S_OK) {
    VARIANT vtAntecedent, vtDependent;
    pClsObj->Get(L"Antecedent", 0, &vtAntecedent, nullptr, nullptr);
    pClsObj->Get(L"Dependent", 0, &vtDependent, nullptr, nullptr);

    const std::wstring dependent = vtDependent.bstrVal;

    if (dependent.find(partitionDeviceID) != std::wstring::npos) {
      std::wstring antecedent = vtAntecedent.bstrVal;
      size_t start = antecedent.find(LR"(DeviceID=")");

      if (start != std::wstring::npos) {
        start += wcslen(LR"(DeviceID=")");
        const size_t end = antecedent.find(L'\"', start);

        if (end != std::wstring::npos) {
          diskDeviceID = antecedent.substr(start, end - start);

          // backslashes are escaped in the returned string,
          // so we need to unescape the string for comparison later
          StringUtil::replaceAll(diskDeviceID, LR"(\\)", LR"(\)");
        }
      }

      VariantClear(&vtAntecedent);
      VariantClear(&vtDependent);
      pClsObj->Release();
      break;
    }

    VariantClear(&vtAntecedent);
    VariantClear(&vtDependent);
    pClsObj->Release();
  }

  pEnum->Release();

  if (diskDeviceID.empty()) {
    pServices->Release();
    return NOHWID_DISK;
  }

  // get the physical disk devices
  hr = pServices->ExecQuery(
      _bstr_t(L"WQL"), _bstr_t(L"SELECT * FROM Win32_PhysicalMedia"),
      WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnum);

  if (FAILED(hr)) {
    pServices->Release();
    return NOHWID_DISK;
  }

  std::wstring serial;

  while (pEnum->Next(WBEM_INFINITE, 1, &pClsObj, &ret) == S_OK) {
    VARIANT vtDeviceID;
    pClsObj->Get(L"Tag", 0, &vtDeviceID, nullptr, nullptr);

    std::wstring tag = vtDeviceID.bstrVal;
    VariantClear(&vtDeviceID);

    if (tag == diskDeviceID) {
      VARIANT vtSerial;

      if (SUCCEEDED(
              pClsObj->Get(L"SerialNumber", 0, &vtSerial, nullptr, nullptr)) &&
          vtSerial.vt == VT_BSTR) {
        serial = vtSerial.bstrVal;
        VariantClear(&vtSerial);
      }

      pClsObj->Release();
      break;
    }

    pClsObj->Release();
  }

  pEnum->Release();
  pServices->Release();

  // convert from wide string for hashing
  const int size = WideCharToMultiByte(CP_UTF8, 0, serial.c_str(), -1, nullptr,
                                       0, nullptr, nullptr);
  std::string result(size - 1, 0); // -1 to discard null terminator
  WideCharToMultiByte(CP_UTF8, 0, serial.c_str(), -1, &result[0], size, nullptr,
                      nullptr);

  return Crypto::sha2(result);
}

    #define CLOSEHANDLE(x)                                                     \
      if (x) {                                                                 \
        CloseHandle(x);                                                        \
      }
    #define LOCALFREE(x)                                                       \
      if (x) {                                                                 \
        LocalFree(x);                                                          \
      }

std::string ETJump::OperatingSystem::getCurrentUserSID() {
  HANDLE hToken = nullptr;

  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
    return NOHWID_SID;
  }

  DWORD dwBufferSize = 0;
  GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwBufferSize);

  if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
    CLOSEHANDLE(hToken)
    return NOHWID_SID;
  }

  const auto pTokenUser =
      static_cast<PTOKEN_USER>(LocalAlloc(LPTR, dwBufferSize));

  if (!pTokenUser) {
    CLOSEHANDLE(hToken)
    return NOHWID_SID;
  }

  if (!GetTokenInformation(hToken, TokenUser, pTokenUser, dwBufferSize,
                           &dwBufferSize)) {
    LOCALFREE(pTokenUser);
    CLOSEHANDLE(hToken);
    return NOHWID_SID;
  }

  LPSTR stringSID = nullptr;

  if (!ConvertSidToStringSid(pTokenUser->User.Sid, &stringSID)) {
    LOCALFREE(pTokenUser);
    CLOSEHANDLE(hToken);
    return NOHWID_SID;
  }

  const std::string result = stringSID;

  LOCALFREE(stringSID);
  LOCALFREE(pTokenUser);
  CLOSEHANDLE(hToken);
  return Crypto::sha2(result);
}

std::string ETJump::OperatingSystem::getSystemUUID() {
  const auto uuid = getWMIProperty(L"Win32_ComputerSystemProduct", L"UUID");

  if (StringUtil::iEqual(uuid, NO_HWID)) {
    return NOHWID_SYS_UUID;
  }

  return Crypto::sha2(uuid);
}

std::string ETJump::OperatingSystem::getMBSerial() {
  const auto mbSerial = getWMIProperty(L"Win32_BaseBoard", L"SerialNumber");

  if (StringUtil::iEqual(mbSerial, NO_HWID)) {
    return NOHWID_MB_SERIAL;
  }

  return Crypto::sha2(mbSerial);
}

std::string
ETJump::OperatingSystem::getWMIProperty(const std::wstring &wmiClass,
                                        const std::wstring &propName) {
  HRESULT hr;
  IWbemServices *pServices = getWMIService();

  if (!pServices) {
    return NOHWID;
  }

  IEnumWbemClassObject *pEnum = nullptr;

  const std::wstring wmiQuery = L"SELECT * FROM " + wmiClass;
  hr = pServices->ExecQuery(
      bstr_t("WQL"), bstr_t(wmiQuery.c_str()),
      WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnum);

  if (FAILED(hr)) {
    pServices->Release();
    return NOHWID;
  }

  IWbemClassObject *pClsObj = nullptr;
  ULONG ret = 0;

  if (pEnum->Next(WBEM_INFINITE, 1, &pClsObj, &ret) != S_OK || ret == 0) {
    pEnum->Release();
    pServices->Release();
    return NOHWID;
  }

  VARIANT vtProp;
  VariantInit(&vtProp);
  std::wstring prop;

  if (SUCCEEDED(pClsObj->Get(propName.c_str(), 0, &vtProp, nullptr, nullptr))) {
    if (vtProp.vt == VT_BSTR && vtProp.bstrVal != nullptr) {
      prop = vtProp.bstrVal;
    }

    VariantClear(&vtProp);
  }

  pClsObj->Release();

  pEnum->Release();
  pServices->Release();

  if (prop.empty()) {
    return NOHWID;
  }

  // convert from wide string for hashing
  const int size = WideCharToMultiByte(CP_UTF8, 0, prop.c_str(), -1, nullptr, 0,
                                       nullptr, nullptr);
  std::string result(size - 1, 0); // -1 to discard null terminator
  WideCharToMultiByte(CP_UTF8, 0, prop.c_str(), -1, &result[0], size, nullptr,
                      nullptr);

  return result;
}

IWbemServices *ETJump::OperatingSystem::getWMIService() {
  IWbemLocator *pLocator = nullptr;
  IWbemServices *pServices = nullptr;

  HRESULT hr =
      CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                       IID_IWbemLocator, reinterpret_cast<LPVOID *>(&pLocator));

  if (FAILED(hr)) {
    return nullptr;
  }

  hr = pLocator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr,
                               nullptr, 0, nullptr, nullptr, &pServices);
  pLocator->Release();

  if (FAILED(hr)) {
    return nullptr;
  }

  hr = CoSetProxyBlanket(pServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                         nullptr, RPC_C_AUTHN_LEVEL_CALL,
                         RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);

  if (FAILED(hr)) {
    pServices->Release();
    return nullptr;
  }

  return pServices;
}

int ETJump::OperatingSystem::getOS() {
    #ifdef _WIN64
  return Constants::OS_WIN_X86_64;
    #else
  // the entire file is inside #ifdef WIN32, so we can just fall back to this
  return Constants::OS_WIN_X86;
    #endif
}

  #else
std::string ETJump::OperatingSystem::getHwid() {
  std::string hardwareId;
  std::string rootDrive;
  DWORD vsn = 0;

  SYSTEM_INFO systemInfo;
  GetSystemInfo(&systemInfo);

  // Random data from processor
  const unsigned int systemInfoSum = systemInfo.dwProcessorType +
                                     systemInfo.wProcessorLevel +
                                     systemInfo.wProcessorArchitecture;

  hardwareId += std::to_string(systemInfoSum);

  // volume serial number
  char buffer[MAX_PATH]{};
  GetEnvironmentVariable("HOMEDRIVE", buffer, sizeof(buffer));
  rootDrive = std::string(buffer) + "\\";

  if (GetVolumeInformation(rootDrive.c_str(), nullptr, 0, &vsn, nullptr,
                           nullptr, nullptr, 0) == 0) {
    // failed to get volume info (noop)
  }

  hardwareId += std::to_string(vsn);
  return Crypto::sha1(hardwareId);
}

  #endif
#endif
