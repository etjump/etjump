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
  #include "../game/etj_filesystem.h"
  #include "../game/etj_string_utilities.h"
  #include "../game/etj_json_utilities.h"

  #include <uuid4.h>

namespace ETJump {
ClientAuth::ClientAuth() {
  serverCommandsHandler->subscribe(
      Constants::Authentication::GUID_REQUEST,
      [&](const std::vector<std::string> &) { guidResponse(); });

  serverCommandsHandler->subscribe(
      Constants::Authentication::GUID_MIGRATE_REQUEST,
      [&](const std::vector<std::string> &) { migrationResponse(); });

  if (!FileSystem::exists(AUTH_FILE)) {
    createAuthFile();
  }

  if (FileSystem::exists(GUID_FILE_OLD) &&
      getGuid(GUIDVersion::GUID_V1).empty()) {
    migrateOldGuid();
  }
}

ClientAuth::~ClientAuth() {
  serverCommandsHandler->unsubscribe(Constants::Authentication::GUID_REQUEST);
  serverCommandsHandler->unsubscribe(
      Constants::Authentication::GUID_MIGRATE_REQUEST);
}

void ClientAuth::guidResponse() {
  const std::string guid = getGuid(GUIDVersion::GUID_V2);
  const int os = getOS();
  const std::vector<std::string> hwid = getHwid();

  const std::string authMsg =
      stringFormat("%s %s %i %s", Constants::Authentication::AUTHENTICATE,
                   Crypto::sha2(guid), os, StringUtil::join(hwid, " "));
  trap_SendClientCommand(authMsg.c_str());
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
 */
void ClientAuth::migrationResponse() {
  const std::string oldGuid = getGuid(GUIDVersion::GUID_V1);

  if (oldGuid.empty()) {
    trap_SendClientCommand(Constants::Authentication::GUID_MIGRATE_FAIL);
  } else {
    trap_SendClientCommand(va("%s %s", Constants::Authentication::GUID_MIGRATE,
                              Crypto::sha1(oldGuid).c_str()));
  }
}

int ClientAuth::getOS() { return OperatingSystem::getOS(); }

std::vector<std::string> ClientAuth::getHwid() {
  return OperatingSystem::getHwid();
}

std::string ClientAuth::getGuid(const GUIDVersion version) {
  Json::Value root;
  std::string err;

  if (!JsonUtils::readFile(AUTH_FILE, root, &err)) {
    CG_Printf("Unable to get GUID: %s\n", err.c_str());

    // we're not using a temporary GUID if we're trying to get old GUID
    if (version == GUIDVersion::GUID_V2) {
      CG_Printf("Using temporary GUID.\n");
    }

    return "";
  }

  std::string guid = version == GUIDVersion::GUID_V2
                         ? root["GUID"]["v2"].asString()
                         : root["GUID"]["v1"].asString();
  return guid;
}

void ClientAuth::createAuthFile() {
  Json::Value root;
  Json::Value guidArray;

  char buf[UUID4_LEN]{};
  uuid4_init();
  uuid4_generate(buf);

  // we don't know if the old GUID exists yet, so leave this empty
  guidArray["v1"] = "";
  guidArray["v2"] = StringUtil::toUpperCase(buf);

  root["GUID"] = guidArray;

  std::string err;

  if (!JsonUtils::writeFile(AUTH_FILE, root, &err)) {
    CG_Printf("%s\nUsing temporary GUID.\n", err.c_str());
  }
}

void ClientAuth::migrateOldGuid() {
  std::vector<char> contents{};
  const std::string failMsg = "Unable to migrate old GUID to a new file:\n%s\n";

  try {
    const File guidFile(GUID_FILE_OLD);
    contents = guidFile.read();
  } catch (const File::FileIOException &e) {
    CG_Printf(failMsg.c_str(), e.what());
    return;
  }

  if (contents.empty()) {
    CG_Printf(failMsg.c_str(), "Empty GUID file.");
    return;
  }

  Json::Value root;
  std::string err;

  if (!JsonUtils::readFile(AUTH_FILE, root, &err)) {
    CG_Printf(failMsg.c_str(), err.c_str());
    return;
  }

  root["GUID"]["v1"] = std::string(contents.begin(), contents.end());

  if (!JsonUtils::writeFile(AUTH_FILE, root, &err)) {
    CG_Printf(failMsg.c_str(), err.c_str());
  }
}
} // namespace ETJump
#endif
