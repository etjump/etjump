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

#pragma once

#include <optional>

#include "etj_string_utilities.h"
#include "g_local.h"

class Printer {
private:
  template <typename T>
  static int32_t getClientNum(const T client) {
    int32_t clientNum = 0;

    if constexpr (std::is_pointer_v<T>) {
      clientNum = client ? ClientNum(client) : CONSOLE_CLIENT_NUMBER;
    } else if constexpr (std::is_integral_v<T>) {
      clientNum = client;
    } else {
      static_assert(sizeof(T) == 0, "Unsupported parameter type for 'client'");
    }

    assert(clientNum >= CONSOLE_CLIENT_NUMBER && clientNum < MAX_CLIENTS);

    return clientNum;
  }

  template <typename T>
  static void
  sendCenterMessage(const T client, const char *cmd, const std::string &message,
                    std::optional<int32_t> priority = std::nullopt) {
    const int32_t clientNum = getClientNum(client);

    if (priority.has_value()) {
      trap_SendServerCommand(clientNum, va("%s \"%s\" %i\n", cmd,
                                           message.c_str(), priority.value()));
    } else {
      trap_SendServerCommand(clientNum,
                             va("%s \"%s\"\n", cmd, message.c_str()));
    }
  }

public:
  static constexpr size_t BYTES_PER_PACKET = 998;
  static constexpr int32_t CONSOLE_CLIENT_NUMBER = -1;
  static constexpr int32_t ALL_CLIENTS = -1;

  // ---------------------------------------------------------------------------
  // Logging
  // ---------------------------------------------------------------------------

  /**
   * Writes a message to the server log.
   * On dedicated servers, also prints to the server console.
   * Message is split into multiple entries if it exceeds 998 bytes.
   * @param message The message to be logged
   */
  template <typename... Targs>
  static void log(const std::string &message, const Targs &...fargs) {
    const auto splits = StringUtils::wrapWords(
        StringUtils::format(message, fargs...), '\n', BYTES_PER_PACKET);

    for (const auto &split : splits) {
      G_LogPrintf(split.c_str());
    }
  }

  /**
   * Same functionality as 'log' but adds a new line in the end.
   * @param message The message to be logged
   */
  template <typename... Targs>
  static void logLn(const std::string &message, const Targs &...fargs) {
    log(StringUtils::format(message, fargs...) + '\n');
  }

  /**
   * Logs message to the admin log, if one is specified.
   * @param message The message to be logged
   * FIXME: 32-bit time
   */
  template <typename... Targs>
  static void logAdmin(const std::string &message, const Targs &...fargs) {
    if (!level.adminLogFile) {
      return;
    }

    qtime_t tm;
    trap_RealTime(&tm);

    const char *msg = va("%02i:%02i:%02i %s", tm.tm_hour, tm.tm_min, tm.tm_sec,
                         StringUtils::format(message, fargs...).c_str());
    trap_FS_Write(msg, static_cast<int32_t>(std::strlen(msg)),
                  level.adminLogFile);
  }

  /**
   * Same functionality as 'logAdmin' but adds a new line in the end.
   * @param message The message to be logged
   */
  template <typename... Targs>
  static void logAdminLn(const std::string &message, const Targs &...fargs) {
    logAdmin(StringUtils::format(message, fargs...) + '\n');
  }

  // ---------------------------------------------------------------------------
  // Console
  // ---------------------------------------------------------------------------

  /**
   * Prints to client console.
   * Will send multiple messages if the message is longer than 998 bytes.
   * If 'client' is -1, prints to server console.
   * @param client The client to send the message to
   * @param message The message to be sent
   */
  template <typename T, typename... Targs>
  static void console(const T client, const std::string &message,
                      const Targs &...fargs) {
    const int32_t clientNum = getClientNum(client);
    const auto splits = StringUtils::wrapWords(
        StringUtils::format(message, fargs...), '\n', BYTES_PER_PACKET);

    for (const auto &split : splits) {
      if (clientNum == CONSOLE_CLIENT_NUMBER) {
        G_Printf("%s", split.c_str());
      } else {
        trap_SendServerCommand(clientNum, va("print \"%s\"", split.c_str()));
      }
    }
  }

  /**
   * Sends a console message to everyone in the server.
   * Also prints to server console on dedicated servers.
   * Will send multiple messages if the message is longer than 998 bytes.
   * @param message The message to be sent
   */
  template <typename... Targs>
  static void consoleAll(const std::string &message, const Targs &...fargs) {
    const auto splits = StringUtils::wrapWords(
        StringUtils::format(message, fargs...), '\n', BYTES_PER_PACKET);

    for (const auto &split : splits) {
      trap_SendServerCommand(ALL_CLIENTS, va("print \"%s\"", split.c_str()));

      if (g_dedicated.integer) {
        G_Printf("%s", split.c_str());
      }
    }
  }

  // ---------------------------------------------------------------------------
  // Chat
  // ---------------------------------------------------------------------------

  /**
   * Prints to client chat. If 'client' is -1, prints to server console.
   * @param client The client to send the message to
   * @param message The message to be sent
   */
  template <typename T, typename... Targs>
  static void chat(const T client, const std::string &message,
                   const Targs &...fargs) {
    const int32_t clientNum = getClientNum(client);
    const std::string msg = StringUtils::format(message, fargs...);

    if (clientNum == CONSOLE_CLIENT_NUMBER) {
      G_Printf("%s\n", msg.c_str());
    } else {
      trap_SendServerCommand(clientNum, va("chat \"%s\"", msg.c_str()));
    }
  }

  /**
   * Prints to client chat. If 'client' is -1, prints to server console.
   * The message sent will be QP-encoded, and decoded by the client
   * @param client The client to send the message to
   * @param message The message to be sent
   */
  template <typename T, typename... Targs>
  static void chatEnc(const T client, const std::string &message,
                      const Targs &...fargs) {
    const int32_t clientNum = getClientNum(client);
    const std::string msg = StringUtils::format(message, fargs...);

    if (clientNum == CONSOLE_CLIENT_NUMBER) {
      G_Printf("%s\n", msg.c_str());
    } else {
      trap_SendServerCommand(clientNum, va("enc_chat \"%s\"", msg.c_str()));
    }
  }

  /**
   * Sends a chat message to everyone in server.
   * Also prints to server console on dedicated servers.
   * @param message The message to be sent
   */
  template <typename... Targs>
  static void chatAll(const std::string &message, const Targs &...fargs) {
    const std::string msg = StringUtils::format(message, fargs...);
    trap_SendServerCommand(ALL_CLIENTS, va("chat \"%s\"", msg.c_str()));

    if (g_dedicated.integer) {
      G_Printf("%s\n", msg.c_str());
    }
  }

  /**
   * Sends a chat message to everyone in server.
   * The message sent will be QP-encoded, and decoded by the clients
   * Also prints to server console on dedicated servers.
   * @param message The message to be sent
   */
  template <typename... Targs>
  static void chatEncAll(const std::string &message, const Targs &...fargs) {
    const std::string msg = StringUtils::format(message, fargs...);
    trap_SendServerCommand(ALL_CLIENTS, va("enc_chat \"%s\"", msg.c_str()));

    if (g_dedicated.integer) {
      G_Printf("%s\n", msg.c_str());
    }
  }

  // ---------------------------------------------------------------------------
  // Popups
  // ---------------------------------------------------------------------------

  /**
   * Prints a popup message. If 'client' is -1, prints to server console.
   * @param client The client to send the popup to
   * @param message The message to be sent
   */
  template <typename T, typename... Targs>
  static void popup(const T client, const std::string &message,
                    const Targs &...fargs) {
    const int32_t clientNum = getClientNum(client);
    const std::string msg = StringUtils::format(message, fargs...);

    if (clientNum == CONSOLE_CLIENT_NUMBER) {
      G_Printf("%s\n", msg.c_str());
    } else {
      trap_SendServerCommand(clientNum, va("cpm \"%s\"", msg.c_str()));
    }
  }

  /**
   * Sends a popup message to everyone in server.
   * Also prints to server console on dedicated servers.
   * @param message The message to be sent
   */
  template <typename... Targs>
  static void popupAll(const std::string &message, const Targs &...fargs) {
    const std::string msg = StringUtils::format(message, fargs...);
    trap_SendServerCommand(ALL_CLIENTS, va("cpm \"%s\"", msg.c_str()));

    if (g_dedicated.integer) {
      G_Printf("%s\n", msg.c_str());
    }
  }

  // ---------------------------------------------------------------------------
  // Center prints
  // For priority prints, the priority value is a completely arbitrary
  // integer value - the client will simply display the highest priority
  // center print that is active at any given time.
  // ---------------------------------------------------------------------------

  /**
   * Sends a center print message to target client.
   * If client num is -1, sends to all clients.
   * @param client The client to send the message to
   * @param message The message to be sent
   */
  template <typename T, typename... Targs>
  static void center(const T client, const std::string &message,
                     const Targs &...fargs) {
    sendCenterMessage(client, "cp", StringUtils::format(message, fargs...));
  }

  /**
   * Sends a center print message to target client.
   * Message sent via this is not logged by 'etj_logCenterPrint'
   * If client num is -1, sends to all clients.
   * @param client The client to send the message to
   * @param message The message to be sent
   */
  template <typename T, typename... Targs>
  static void centerNoLog(const T client, const std::string &message,
                          const Targs &...fargs) {
    sendCenterMessage(client, "cpnl", StringUtils::format(message, fargs...));
  }

  /**
   * Broadcasts a center print message to all clients.
   * Also prints to server console on dedicated servers.
   * @param message The message to be sent
   */
  template <typename... Targs>
  static void centerAll(const std::string &message, const Targs &...fargs) {
    const std::string msg = StringUtils::format(message, fargs...);
    sendCenterMessage(ALL_CLIENTS, "cp", msg);

    if (g_dedicated.integer) {
      G_Printf("%s\n", msg.c_str());
    }
  }

  /**
   * Broadcasts a center print message to all clients
   * Message sent via this is not logged by 'etj_logCenterPrint'
   * @param message The message to be sent
   */
  template <typename T, typename... Targs>
  static void centerAllNoLog(const std::string &message,
                             const Targs &...fargs) {
    const std::string msg = StringUtils::format(message, fargs...);
    sendCenterMessage(ALL_CLIENTS, "cpnl", msg);

    if (g_dedicated.integer) {
      G_Printf("%s\n", msg.c_str());
    }
  }

  /**
   * Sends a center print message to target client with priority
   * If client num is -1, sends to all clients.
   * @param client The client to send the message to
   * @param priority The priority of the message
   * @param message The message to be sent
   */
  template <typename T, typename... Targs>
  static void centerPriority(const T client, const int32_t priority,
                             const std::string &message,
                             const Targs &...fargs) {
    sendCenterMessage(client, "cp", StringUtils::format(message, fargs...),
                      priority);
  }

  /**
   * Sends a center print message to target client.
   * Message sent via this is not logged by 'etj_logCenterPrint'
   * If client num is -1, sends to all clients.
   * @param client The client to send the message to
   * @param priority The priority of the message
   * @param message The message to be sent
   */
  template <typename T, typename... Targs>
  static void centerPriorityNoLog(const T client, int32_t priority,
                                  const std::string &message,
                                  const Targs &...fargs) {
    sendCenterMessage(client, "cpnl", StringUtils::format(message, fargs...),
                      priority);
  }

  /**
   * Broadcasts a center print message to all clients with priority.
   * Also prints to server console on dedicated servers.
   * @param priority The priority of the message
   * @param message The message to be sent
   */
  template <typename... Targs>
  static void centerPriorityAll(const int32_t priority,
                                const std::string &message,
                                const Targs &...fargs) {
    const std::string msg = StringUtils::format(message, fargs...);
    sendCenterMessage(ALL_CLIENTS, "cp", msg, priority);

    if (g_dedicated.integer) {
      G_Printf("%s\n", msg.c_str());
    }
  }

  /**
   * Broadcasts a center print message to all clients with priority.
   * Message sent via this is not logged by 'etj_logCenterPrint'
   * Also prints to server console on dedicated servers.
   * @param priority The priority of the message
   * @param message The message to be sent
   */
  template <typename... Targs>
  static void centerPriorityAllNoLog(const int32_t priority,
                                     const std::string &message,
                                     const Targs &...fargs) {
    const std::string msg = StringUtils::format(message, fargs...);
    sendCenterMessage(ALL_CLIENTS, "cpnl", msg, priority);

    if (g_dedicated.integer) {
      G_Printf("%s\n", msg.c_str());
    }
  }

  // ---------------------------------------------------------------------------
  // Banner prints
  // ---------------------------------------------------------------------------

  /**
   * Sends a banner message to a client.
   * If 'client' is -1,  sends to console.
   * @param client The client to send the message to
   * @param message The message to be sent
   */
  template <typename T, typename... Targs>
  static void banner(const T client, const std::string &message,
                     const Targs &...fargs) {
    const int32_t clientNum = getClientNum(client);
    const std::string msg = StringUtils::format(message, fargs...);

    if (clientNum == CONSOLE_CLIENT_NUMBER) {
      G_Printf("%s\n", msg.c_str());
    } else {
      trap_SendServerCommand(clientNum, va("bp \"%s\"", msg.c_str()));
    }
  }

  /**
   * Sends a banner message to all clients.
   * Also prints to server console on dedicated servers.
   * @param message The message to be sent
   */
  template <typename... Targs>
  static void bannerAll(const std::string &message, const Targs &...fargs) {
    const std::string msg = StringUtils::format(message, fargs...);
    trap_SendServerCommand(ALL_CLIENTS, va("bp \"%s\"", msg.c_str()));

    if (g_dedicated.integer) {
      G_Printf("%s\n", msg.c_str());
    }
  }

  // ---------------------------------------------------------------------------
  // Commands
  // TODO: move this stuff elsewhere!
  // ---------------------------------------------------------------------------

  /**
   * Sends a command to client.
   * @param clientNum The client to send the command to
   * @param command The command to be sent
   */
  static void command(const int32_t clientNum, const std::string &command) {
    trap_SendServerCommand(clientNum, command.c_str());
  }

  /**
   * Sends a command to multiple clients.
   * @param clientNums The clients to send the command to
   * @param command The command to be sent
   */
  static void command(const std::vector<int> &clientNums,
                      const std::string &command) {
    for (const auto &clientNum : clientNums) {
      trap_SendServerCommand(clientNum, command.c_str());
    }
  }

  /**
   * Sends a command to all clients.
   * @param command The command to be sent
   */
  static void commandAll(const std::string &command) {
    trap_SendServerCommand(ALL_CLIENTS, command.c_str());
  }
};
