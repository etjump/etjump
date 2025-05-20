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
inline constexpr char MIGRATE_CMD[] = "migrateGuid";

ClientAuth::ClientAuth() {
  serverCommandsHandler->subscribe(
      Constants::Authentication::AUTH_REQUEST,
      [&](const std::vector<std::string> &) { guidResponse(); });

  serverCommandsHandler->subscribe(
      Constants::Authentication::GUID_MIGRATE_REQUEST,
      [&](const std::vector<std::string> &) { migrationResponse(); });

  consoleCommandsHandler->subscribe(
      MIGRATE_CMD, [&](const std::vector<std::string> &args) {
        if (!args.empty() && (args[0] == "-f" || args[0] == "--force")) {
          manualMigration(
              Constants::Authentication::MigrationType::MANUAL_FORCE);
        } else {
          manualMigration(Constants::Authentication::MigrationType::MANUAL);
        }
      });

  if (!FileSystem::exists(AUTH_FILE)) {
    createAuthFile();
  }

  if (FileSystem::exists(GUID_FILE_OLD) &&
      getGuid(GUIDVersion::GUID_V1).empty()) {
    migrateOldGuid();
  }
}

ClientAuth::~ClientAuth() {
  serverCommandsHandler->unsubscribe(Constants::Authentication::AUTH_REQUEST);
  serverCommandsHandler->unsubscribe(
      Constants::Authentication::GUID_MIGRATE_REQUEST);
  consoleCommandsHandler->unsubscribe(MIGRATE_CMD);
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

void ClientAuth::migrationResponse() {
  const std::string oldGuid = getGuid(GUIDVersion::GUID_V1);

  if (oldGuid.empty()) {
    trap_SendClientCommand(Constants::Authentication::GUID_MIGRATE_FAIL);
  } else {
    trap_SendClientCommand(
        va("%s %s %i", Constants::Authentication::GUID_MIGRATE,
           Crypto::sha1(oldGuid).c_str(),
           static_cast<int>(Constants::Authentication::MigrationType::AUTO)));
  }
}

void ClientAuth::manualMigration(
    const Constants::Authentication::MigrationType type) {
  if (!FileSystem::exists(GUID_FILE_OLD)) {
    CG_Printf("Old GUID file not found. Make sure ^3'%s' ^7is present in "
              "^3'etjump' ^7directory.\n",
              GUID_FILE_OLD);
    return;
  }

  // user manually deleted their auth file on runtime...
  if (!FileSystem::exists(AUTH_FILE)) {
    CG_Printf(
        "Authentication file ^3'etjump/%s' ^7not found. Make sure the file "
        "exists, or reconnect to create a new one.\n",
        AUTH_FILE);
    return;
  }

  // TODO: this should probably parse 'etguid.dat' again,
  //  so the user doesn't need to manually edit 'auth.dat'
  //  if they are performing a forced re-migration
  std::string oldGuid = getGuid(GUIDVersion::GUID_V1);

  // normally the GUID shouldn't be empty here since we auto-migrate on init,
  // but it can happen in two scenarios:
  // 1. the old GUID wasn't present on init, but the user later dropped in
  //    the file to 'etjump' directory during runtime
  // 2. the user manually edited the auth file and erased the old GUID string
  if (!oldGuid.empty()) {
    trap_SendClientCommand(
        va("%s %s %i", Constants::Authentication::GUID_MIGRATE,
           Crypto::sha1(oldGuid).c_str(), static_cast<int>(type)));
  } else {
    migrateOldGuid();
    oldGuid = getGuid(GUIDVersion::GUID_V1);

    // parsing failed
    if (oldGuid.empty()) {
      CG_Printf("Failed to perform GUID migration.\n");
    } else {
      trap_SendClientCommand(
          va("%s %s %i", Constants::Authentication::GUID_MIGRATE,
             Crypto::sha1(oldGuid).c_str(), static_cast<int>(type)));
    }
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
