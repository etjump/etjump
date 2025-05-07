/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
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

#ifdef NEW_AUTH
  #include "etj_client_auth.h"
  #include "cg_local.h"
  #include "etj_client_commands_handler.h"
  #include "etj_operating_system.h"

  #include "../game/etj_shared.h"
  #include "../game/etj_file.h"
  #include "../game/etj_string_utilities.h"

  #include <uuid4.h>

namespace ETJump {
inline constexpr char GUID_FILE[] = "etguid.dat";

ClientAuth::ClientAuth() {
  serverCommandsHandler->subscribe(
      Constants::Authentication::GUID_REQUEST,
      [&](const std::vector<std::string> &) { login(); });
}

ClientAuth::~ClientAuth() {
  serverCommandsHandler->unsubscribe(Constants::Authentication::GUID_REQUEST);
}

void ClientAuth::login() {
  const int os = getOS();
  const std::string guid = getGuid();
  const std::vector<std::string> hwid = getHwid();

  sendAuthResponse(os, guid, hwid);
}

void ClientAuth::sendAuthResponse(int os, const std::string &guid,
                                  const std::vector<std::string> &hwid) {
  const std::string authMsg =
      stringFormat("%s %s %i %s", Constants::Authentication::AUTHENTICATE,
                   hash(guid), os, StringUtil::join(hwid, ","));
  trap_SendClientCommand(authMsg.c_str());
}

int ClientAuth::getOS() { return OperatingSystem::getOS(); }

std::string ClientAuth::getGuid() {
  try {
    const File guidFile(GUID_FILE);
    const auto contents = guidFile.read();
    return {contents.begin(), contents.end()};
  } catch (const File::FileIOException &) {
    char buf[UUID4_LEN]{};
    uuid4_init();
    uuid4_generate(buf);

    const std::string guid = StringUtil::toUpperCase(buf);
    saveGuid(guid);
    return guid;
  }
}

std::vector<std::string> ClientAuth::getHwid() {
  return OperatingSystem::getHwid();
}

void ClientAuth::saveGuid(const std::string &guid) {
  try {
    const File guidFile(GUID_FILE, File::Mode::Write);
    guidFile.write(guid);
  } catch (const File::FileIOException &e) {
    CG_Printf(S_COLOR_YELLOW
              "WARNING: failed to save GUID: %s\nUsing temporary GUID.\n",
              e.what());
  }
}
} // namespace ETJump
#endif
