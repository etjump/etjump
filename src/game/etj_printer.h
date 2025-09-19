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

#pragma once

#include <string>
#include <vector>

struct gentity_s;
struct gclient_s;

using gentity_t = gentity_s;
using gclient_t = gclient_s;

class Printer {
public:
  static constexpr int CONSOLE_CLIENT_NUMBER = -1;
  static constexpr int ALL_CLIENTS = -1;

  /**
   * Prints the message to server console and log. Will
   * send multiple messages if the message is longer than 1000 bytes
   * @param message The message to be sent
   */
  static void log(const std::string &message);

  /**
   * Same functionality as log but adds a new line in the end
   * @param message The message to be sent
   */
  static void logLn(const std::string &message);

  /**
   * Logs message to the admin log.
   * @param message The message to be logged
   */
  static void logAdmin(const std::string &message);

  /**
   * Same functionality as logAdmin but adds a new line in the end
   * @param message The message to be logged
   */
  static void logAdminLn(const std::string &message);

  /**
   * Prints to client console.
   * Will send multiple messages if the message is longer than 998 bytes.
   * If client num is -1 sends to console.
   * If pointer in overloads is nullptr, prints to console.
   * @param clientNum The client to send the message to
   * @param message The message to be sent
   */
  static void console(int clientNum, const std::string &message);
  static void console(const gentity_t *ent, const std::string &message);
  static void console(const gclient_t *client, const std::string &message);

  /**
   * Sends a console message to everyone in the server and to server
   * console. Will send multiple messages if the message is longer than
   * 998 bytes.
   * @param message The message to be sent
   */
  static void consoleAll(const std::string &message);

  /**
   * Prints to client chat. If client num is -1 sends to console.
   * If pointer in overloads is nullptr, prints to console.
   * @param clientNum The client to send the message to
   * @param message The message to be sent
   */
  static void chat(int clientNum, const std::string &message);
  static void chat(const gentity_t *ent, const std::string &message);
  static void chat(const gclient_t *client, const std::string &message);

  /**
   * Sends a chat message to everyone in server and to server console.
   * @param message The message to be sent
   */
  static void chatAll(const std::string &message);

  /**
   * Prints a popup message. If client num is -1 sends to console
   * If pointer in overloads is nullptr, prints to console.
   * @param clientNum The client to send the message to
   * @param message The message to be sent
   */
  static void popup(int clientNum, const std::string &message);
  static void popup(const gentity_t *ent, const std::string &message);
  static void popup(const gclient_t *client, const std::string &message);

  /**
   * Sends a popup message to everyone in server and to server console.
   * @param message The message to be sent
   */
  static void popupAll(const std::string &message);

  /**
   * Sends a center print message to target client.
   * If client num is -1, sends to all clients.
   * If log is true, the client will log the message to console,
   * if 'etj_logCenterPrint' is set
   * If pointer in overloads is nullptr, prints to console.
   * @param clientNum
   * @param message
   */
  static void center(int clientNum, const std::string &message,
                     bool log = true);
  static void center(const gentity_t *ent, const std::string &message,
                     bool log = true);
  static void center(const gclient_t *client, const std::string &message,
                     bool log = true);

  /**
   * Broadcasts a center print message to all clients
   * @param message The message
   */
  static void centerAll(const std::string &message, bool log = true);

  /**
   * Sends a center print message to target client with priority.
   * If client num is -1, sends to all clients.
   * If log is true, the client will log the message to console,
   * if 'etj_logCenterPrint' is set
   * If pointer in overloads is nullptr, prints to console.
   * @param clientNum
   * @param message
   * @param priority
   * @param log
   */
  static void centerPriority(int clientNum, const std::string &message,
                             int priority, bool log = true);
  static void centerPriority(const gentity_t *ent, const std::string &message,
                             int priority, bool log = true);
  static void centerPriority(const gclient_t *client,
                             const std::string &message, int priority,
                             bool log = true);

  /**
   * Broadcasts a center print message to all clients with priority
   * @param message The message
   * @param priority
   * @param log
   */
  static void centerPriorityAll(const std::string &message, int priority,
                                bool log = true);

  /**
   * Sends a banner message to the client. If client num is -1 sends to console.
   * If pointer in overloads is nullptr, prints to console.
   * @param clientNum the client slot number to send the message to
   * @param message The message to be sent
   */
  static void banner(int clientNum, const std::string &message);
  static void banner(const gentity_t *ent, const std::string &message);
  static void banner(const gclient_t *client, const std::string &message);

  /**
   * Sends a banner print message to all clients.
   * @param message The message to be sent
   */
  static void bannerAll(const std::string &message);

  /**
   * Sends a command to client specified by clientNum. This should
   * probably be elsewhere but is here for now.
   * @param clientNum The client to send the message to
   * @param command The command to be sent
   */
  static void command(int clientNum, const std::string &command);
  static void command(const std::vector<int> &clientNums,
                      const std::string &command);

  /**
   * Sends a command to all clients
   * @param command The command to send to users
   */
  static void commandAll(const std::string &command);
};
