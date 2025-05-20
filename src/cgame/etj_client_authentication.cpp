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

#ifndef NEW_AUTH

  #include "etj_client_authentication.h"
  #include "../game/etj_file.h"
  #include "../game/etj_shared.h"
  #include "../game/etj_string_utilities.h"
  #include "../game/etj_crypto.h"
  #include <uuid4.h>

ETJump::ClientAuthentication::ClientAuthentication(
    std::function<void(const std::string &)> sendClientCommand,
    std::function<void(const std::string &)> print,
    std::function<std::string()> getHwid,
    std::shared_ptr<ClientCommandsHandler> serverCommandsHandler)
    : _sendClientCommand(sendClientCommand), _print(print), _getHwid(getHwid),
      _serverCommandsHandler(serverCommandsHandler), GUID_FILE("etguid.dat") {
  _serverCommandsHandler->subscribe(
      Constants::Authentication::GUID_REQUEST,
      [&](const std::vector<std::string> &args) { login(); });
}

ETJump::ClientAuthentication::~ClientAuthentication() {
  _serverCommandsHandler->unsubscribe(Constants::Authentication::GUID_REQUEST);
}

void ETJump::ClientAuthentication::login() {
  auto guid = getGuid();
  auto result = saveGuid(guid);
  if (!result.success) {
    _print("^1Error: " + result.message);
  }
  auto hwid = getHwid();
  auto authenticate =
      stringFormat("%s %s %s", Constants::Authentication::AUTHENTICATE,
                   Crypto::sha1(guid), hwid);
  _sendClientCommand(authenticate);
}

std::string ETJump::ClientAuthentication::getGuid() {
  try {
    File guidFile(GUID_FILE);
    auto guid = guidFile.read();
    return std::string(begin(guid), end(guid));
  } catch (const File::FileIOException &) {
    return createGuid();
  }
}

std::string ETJump::ClientAuthentication::getHwid() { return _getHwid(); }

std::string ETJump::ClientAuthentication::createGuid() const {
  char buf[UUID4_LEN];
  uuid4_init();
  uuid4_generate(buf);
  auto guid = ETJump::StringUtil::toUpperCase(buf);
  return guid;
}

ETJump::ClientAuthentication::OperationResult
ETJump::ClientAuthentication::saveGuid(const std::string &guid) const {
  try {
    const File guidFile(GUID_FILE, File::Mode::Write);
    guidFile.write(guid);
    return {true, ""};
  } catch (const File::FileIOException &e) {
    return {false, std::string("saving guid failed: ") + e.what() +
                       " Using temporary guid."};
  }
}
#endif
