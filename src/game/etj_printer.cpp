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

#include <utility>

#include "etj_printer.h"
#include "etj_string_utilities.h"
#include "g_local.h"

void Printer::log(const std::string &message) {
  const auto splits = ETJump::wrapWords(message, '\n', BYTES_PER_PACKET);

  for (const auto &split : splits) {
    G_LogPrintf(split.c_str());
  }
}

void Printer::logLn(const std::string &message) { log(message + "\n"); }

void Printer::console(int clientNum, const std::string &message) {
  const auto splits = ETJump::wrapWords(message, '\n', BYTES_PER_PACKET);

  for (const auto &split : splits) {
    if (clientNum == CONSOLE_CLIENT_NUMBER) {
      G_Printf("%s", split.c_str());
    } else {
      trap_SendServerCommand(clientNum, va("print \"%s\"", split.c_str()));
    }
  }
}

void Printer::console(gentity_t *ent, const std::string &message) {
  const int clientNum = ent ? ClientNum(ent) : CONSOLE_CLIENT_NUMBER;
  console(clientNum, message);
}

void Printer::console(gclient_t *client, const std::string &message) {
  const int clientNum = client ? ClientNum(client) : CONSOLE_CLIENT_NUMBER;
  console(clientNum, message);
}

void Printer::consoleAll(const std::string &message) {
  const auto splits = ETJump::wrapWords(message, '\n', BYTES_PER_PACKET);

  for (const auto &split : splits) {
    trap_SendServerCommand(-1, va("print \"%s\"", split.c_str()));
    G_Printf("%s", split.c_str());
  }
}

void Printer::chat(int clientNum, const std::string &message) {
  if (clientNum == CONSOLE_CLIENT_NUMBER) {
    G_Printf("%s\n", message.c_str());
  } else {
    trap_SendServerCommand(clientNum, va("chat \"%s\"", message.c_str()));
  }
}

void Printer::chat(gentity_t *ent, const std::string &message) {
  const int clientNum = ent ? ClientNum(ent) : CONSOLE_CLIENT_NUMBER;
  chat(clientNum, message);
}

void Printer::chat(gclient_t *client, const std::string &message) {
  const int clientNum = client ? ClientNum(client) : CONSOLE_CLIENT_NUMBER;
  chat(clientNum, message);
}

void Printer::chatAll(const std::string &message) {
  trap_SendServerCommand(-1, va("chat \"%s\"", message.c_str()));
  G_Printf("%s\n", message.c_str());
}

void Printer::popup(int clientNum, const std::string &message) {
  if (clientNum == CONSOLE_CLIENT_NUMBER) {
    G_Printf("%s\n", message.c_str());
  } else {
    trap_SendServerCommand(clientNum, va("cpm \"%s\"", message.c_str()));
  }
}

void Printer::popup(gentity_t *ent, const std::string &message) {
  const int clientNum = ent ? ClientNum(ent) : CONSOLE_CLIENT_NUMBER;
  popup(clientNum, message);
}

void Printer::popup(gclient_t *client, const std::string &message) {
  const int clientNum = client ? ClientNum(client) : CONSOLE_CLIENT_NUMBER;
  popup(clientNum, message);
}

void Printer::popupAll(const std::string &message) {
  trap_SendServerCommand(-1, va("cpm \"%s\n\"", message.c_str()));
  G_Printf("%s\n", message.c_str());
}

void Printer::center(int clientNum, const std::string &message) {
  trap_SendServerCommand(clientNum,
                         ETJump::stringFormat("cp \"%s\n\"", message).c_str());
}

void Printer::center(gentity_t *ent, const std::string &message) {
  const int clientNum = ent ? ClientNum(ent) : CONSOLE_CLIENT_NUMBER;
  center(clientNum, message);
}

void Printer::center(gclient_t *client, const std::string &message) {
  const int clientNum = client ? ClientNum(client) : CONSOLE_CLIENT_NUMBER;
  center(clientNum, message);
}

void Printer::centerAll(const std::string &message) {
  trap_SendServerCommand(-1, va("cp \"%s\n\"", message.c_str()));
  G_Printf("%s\n", message.c_str());
}

void Printer::banner(int clientNum, const std::string &message) {
  if (clientNum == CONSOLE_CLIENT_NUMBER) {
    G_Printf("%s\n", message.c_str());
  } else {
    trap_SendServerCommand(clientNum, va("bp \"%s\n\"", message.c_str()));
  }
}

void Printer::banner(gentity_t *ent, const std::string &message) {
  const int clientNum = ent ? ClientNum(ent) : CONSOLE_CLIENT_NUMBER;
  banner(clientNum, message);
}

void Printer::banner(gclient_t *client, const std::string &message) {
  const int clientNum = client ? ClientNum(client) : CONSOLE_CLIENT_NUMBER;
  banner(clientNum, message);
}

void Printer::bannerAll(const std::string &message) {
  trap_SendServerCommand(-1, va("bp \"%s\n\"", message.c_str()));
  G_Printf("%s\n", message.c_str());
}

void Printer::command(int clientNum, const std::string &command) {
  trap_SendServerCommand(clientNum, command.c_str());
}

void Printer::command(const std::vector<int> &clientNums,
                      const std::string &command) {
  for (const auto &clientNum : clientNums) {
    trap_SendServerCommand(clientNum, command.c_str());
  }
}

void Printer::commandAll(const std::string &command) {
  trap_SendServerCommand(-1, command.c_str());
}
