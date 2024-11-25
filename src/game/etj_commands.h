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

#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "utilities.hpp"
#include <map>
#include <functional>

class Commands {
public:
  Commands();
  bool ClientCommand(gentity_t *ent, const std::string &command);
  bool AdminCommand(gentity_t *ent);
  bool List(gentity_t *ent);
  char FindCommandFlag(const std::string &command);
  void ListCommandFlags(gentity_t *ent);

private:
  std::map<std::string, std::function<bool(gentity_t *ent, Arguments argv)>>
      commands_;
  std::map<std::string,
           std::pair<std::function<bool(gentity_t *ent, Arguments argv)>, char>>
      adminCommands_;
  typedef std::map<
      std::string,
      std::pair<std::function<bool(gentity_t *ent, Arguments argv)>,
                char>>::const_iterator AdminCommandsIterator;
};

namespace CommandFlags {
// Silent command execution via '/' flag
// Not listed here because it's not a command in itself

constexpr char BAN = 'b';
// For everyone
constexpr char BASIC = 'a';
constexpr char CANCELVOTE = 'C';
// const char EBALL       = '8';
constexpr char EDIT = 'A';
constexpr char FINGER = 'f';
// const char HELP        = 'h';
constexpr char KICK = 'k';
constexpr char LISTBANS = 'L';
constexpr char LISTPLAYERS = 'l';
constexpr char MAP = 'M';
constexpr char MUTE = 'm';
constexpr char NOCLIP = 'N';
constexpr char PASSVOTE = 'P';
// const char READCONFIG  = 'G';
constexpr char RENAME = 'R';
constexpr char RESTART = 'r';
constexpr char TOKENS = 'V';
constexpr char SETLEVEL = 's';
constexpr char MOVERSCALE = 'v';
constexpr char TIMERUN_MANAGEMENT = 'T';
constexpr char CUSTOMVOTES = 'c';
constexpr char ADMINCHAT = 'S';

// usage of admin commmands with these flags gets logged to g_adminLog
constexpr char loggedCommandFlags[] = {
    BAN,      CANCELVOTE,         EDIT,        KICK, MUTE, PASSVOTE, RENAME,
    SETLEVEL, TIMERUN_MANAGEMENT, CUSTOMVOTES,
};

} // namespace CommandFlags

#endif
