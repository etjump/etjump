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
  #include "../game/etj_crypto.h"

  #include "../game/etj_shared.h"
  #include "../game/etj_file.h"
  #include "../game/etj_string_utilities.h"

  #include <iostream>
  #include <uuid4.h>

namespace ETJump {
inline constexpr char GUID_FILE_V1[] = "etguid.dat";
inline constexpr char GUID_FILE_V2[] = "auth/guid.dat";

inline constexpr char GUID_PREFIX_V1[] = "[V1]";
inline constexpr char GUID_PREFIX_V2[] = "[V2]";
inline constexpr size_t GUID_PREFIX_LEN = 4;

ClientAuth::ClientAuth() {
  serverCommandsHandler->subscribe(
      Constants::Authentication::GUID_REQUEST,
      [&](const std::vector<std::string> &) { login(); });

  serverCommandsHandler->subscribe(
      Constants::Authentication::GUID_MIGRATE_REQUEST,
      [&](const std::vector<std::string> &) { migrateGuid(); });
}

ClientAuth::~ClientAuth() {
  serverCommandsHandler->unsubscribe(Constants::Authentication::GUID_REQUEST);
  serverCommandsHandler->unsubscribe(
      Constants::Authentication::GUID_MIGRATE_REQUEST);
}

void ClientAuth::login() {
  const int os = getOS();
  const std::string guid = getGuid(GUIDVersion::GUID_V2);
  const std::vector<std::string> hwid = getHwid();

  sendAuthResponse(os, guid, hwid);
}

void ClientAuth::sendAuthResponse(int os, const std::string &guid,
                                  const std::vector<std::string> &hwid) {
  const std::string authMsg =
      stringFormat("%s %s %i %s", Constants::Authentication::AUTHENTICATE,
                   Crypto::sha2(guid), os, StringUtil::join(hwid, " "));
  trap_SendClientCommand(authMsg.c_str());
}

int ClientAuth::getOS() { return OperatingSystem::getOS(); }

std::string ClientAuth::getGuid(const GUIDVersion version) {
  std::vector<char> contents;

  try {
    const File guidFile(getGuidFileName(version));
    contents = guidFile.read();
  } catch (const File::FileIOException &) {
    return version == GUIDVersion::GUID_V2 ? createGuid() : "";
  }

  // never create old GUID files anymore
  if (version == GUIDVersion::GUID_V1) {
    return {contents.begin(), contents.end()};
  }

  const auto guids =
      StringUtil::split({contents.cbegin(), contents.cend()}, "\n");

  for (const auto &guid : guids) {
    if (StringUtil::startsWith(guid, GUID_PREFIX_V2)) {
      return guid.substr(GUID_PREFIX_LEN, guid.length() - GUID_PREFIX_LEN);
    }
  }

  return createGuid();
}

std::vector<std::string> ClientAuth::getHwid() {
  return OperatingSystem::getHwid();
}

void ClientAuth::saveGuid(const std::string &contents) {
  try {
    const File guidFile(GUID_FILE_V2, File::Mode::Write);
    guidFile.write(contents);
  } catch (const File::FileIOException &e) {
    CG_Printf(S_COLOR_YELLOW
              "WARNING: failed to save GUID: %s\nUsing temporary GUID.\n",
              e.what());
  }
}

std::string ClientAuth::createGuid() {
  std::string contents;
  const std::string oldGuid = getGuid(GUIDVersion::GUID_V1);

  if (oldGuid.empty()) {
    contents += std::string(GUID_PREFIX_V1) + "NOGUID\n";
  } else {
    contents += GUID_PREFIX_V1 + oldGuid + '\n';
  }

  char buf[UUID4_LEN]{};
  uuid4_init();
  uuid4_generate(buf);
  const std::string guid = StringUtil::toUpperCase(buf);

  contents += GUID_PREFIX_V2 + guid;
  saveGuid(contents);

  return guid;
}

/*
 * TODO: we might want to have a way to perform this manually.
 *  Consider the following scenario: a user has reinstalled ET, and has their
 *  'etguid.dat' backed up. However, they forget to put the file into etjump
 *  directory before connecting to a server, so a migration attempt will fail.
 *  The server now has their new GUID in the database, and future connections
 *  will authenticate using the new GUID, meaning it's now impossible to
 *  restore identity using the old GUID without manual intervention from server.
 *  To mitigate this, a client should be able to use a command to perform
 *  a migration on-demand, so migration can be done after the client
 *  has already visited a server once.
 *
 *  TODO: Additionally, we should probably migrate the old GUID value into
 *   the new GUID file in the above scenario, right now if the above happens,
 *   the old GUID in the new file will just stay as '[V1]NOGUID' even if
 *   the old file is later added.
 */
void ClientAuth::migrateGuid() {
  const std::string oldGuid = getGuid(GUIDVersion::GUID_V1);

  // TODO: check if new GUID file contains the old GUID
  //  and migrate using that if found
  if (oldGuid.empty()) {
    trap_SendClientCommand(Constants::Authentication::GUID_MIGRATE_FAIL);
  } else {
    trap_SendClientCommand(va("%s %s", Constants::Authentication::GUID_MIGRATE,
                              Crypto::sha1(oldGuid).c_str()));
  }
}

std::string ClientAuth::getGuidFileName(const GUIDVersion version) {
  return version == GUIDVersion::GUID_V2 ? GUID_FILE_V2 : GUID_FILE_V1;
}
} // namespace ETJump
#endif
