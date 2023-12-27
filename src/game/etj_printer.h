/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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

#ifndef ETJUMP_PRINTER_H
#define ETJUMP_PRINTER_H

#ifdef min
  #undef min
#endif
#ifdef max
  #undef max
#endif

#include <string>
#include <vector>

class Printer {
public:
  static const int CONSOLE_CLIENT_NUMBER = -1;
  /**
   * Prints the message to server console
   */
  static void PrintLn(std::string message);
  /**
   * Prints the message to server console and log. Will
   * send multiple messages if the message is longer than 1000 bytes
   * @param message The message to be sent
   */
  static void LogPrint(std::string message);

  /**
   * Same functionality as LogPrint but adds a new line in the end
   * @param message The message to be sent
   */
  static void LogPrintln(const std::string &message);

  /**
   * Prints to client console. Will send multiple messages if the
   * message is longer than 1000 bytes.
   * If client num is -1 sends to console
   * @param clientNum The client to send the message to
   * @param message The message to be sent
   */
  static void SendConsoleMessage(int clientNum, std::string message);

  /**
   * Sends a console message to everyone in the server and to server
   * console. Will send multiple messages if the message is longer than
   * 1000 bytes.
   * @param message The message to be sent
   */
  static void BroadcastConsoleMessage(std::string message);

  /**
   * Prints to client chat. If client num is -1 sends to console
   * @param clientNum The client to send the message to
   * @param message The message to be sent
   */
  static void SendChatMessage(int clientNum, const std::string &message);

  /**
   * Prints a popup message. If client num is -1 sends to console
   * @param clientNum The client to send the message to
   * @param message The message to be sent
   */
  static void SendPopupMessage(int clientNum, const std::string &message);

  /**
   * Sends a chat message to everyone in server and to server console.
   * @param message The message to be sent
   */
  static void BroadcastChatMessage(const std::string &message);

  /**
   * Sends a popup message to everyone in server and to server console.
   * @param message The message to be sent
   */
  static void BroadcastPopupMessage(const std::string &message);

  /**
   * Sends a banner message to the client.
   * @param clientNum the client slot number to send the message to
   * @param message The message to be sent
   */
  static void SendBannerMessage(int clientNum, const std::string &message);

  /**
   * Sends a banner print message to all clients.
   * @param message The message to be sent
   */
  static void BroadcastLeftBannerMessage(const std::string &message);
  /**
   * Sends a banner print message to all clients.
   * @param message The message to be sent
   */
  static void BroadCastBannerMessage(const std::string &message);
  /**
   * Sends a command to client specified by clientNum. This should
   * probably be elsewhere but is here for now.
   * @param clientNum The client to send the message to
   * @param command The command to be sent
   */
  static void SendCommand(int clientNum, const std::string &command);

  /**
   * Sends a command to all clients specified in the clientNums vector.
   * @param clientNums The list of client numbers to send the command to
   * @param command The command to send to users
   */
  static void SendCommand(std::vector<int> clientNums,
                          const std::string &command);

  /**
   * Sends a command to all clients
   * @param command The command to send to users
   */
  static void SendCommandToAll(const std::string &command);

  /**
   * Broadcasts a top banner message to all clients
   * @param message The message
   */
  static void BroadcastTopBannerMessage(const std::string &message);

  /**
   * Broadcasts a left banner message to all clients
   * @param message The message
   */
  static void BroadcastCenterMessage(const std::string &message);
  /**
   * Sends a center print message to target client
   * @param clientNum
   * @param message
   */
  static void SendCenterMessage(int clientNum, const std::string &message);

private:
};

#endif // ETJUMP_PRINTER_H
