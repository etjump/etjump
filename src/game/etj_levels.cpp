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

#include "etj_levels.h"
#include "etj_local.h"
#include "etj_printer.h"
#include "etj_string_utilities.h"

Levels::Level::Level(int level, std::string const &name,
                     std::string const &greeting, std::string const &commands) {
  this->level = level;
  this->name = name;
  this->greeting = greeting;
  this->commands = commands;
}

Levels::Levels() {
  dummyLevel_ = std::shared_ptr<Level>(new Level(0, "", "", ""));
}

Levels::~Levels() {}

bool Levels::Add(int level, std::string const name, std::string const commands,
                 std::string const greeting) {
  auto it = FindConst(level);
  if (it != levels_.end()) {
    errorMessage = "level exists";
    return false;
  }

  std::shared_ptr<Level> levelPtr(new Level(level, name, greeting, commands));
  levels_.push_back(levelPtr);

  if (!WriteToConfig()) {
    return false;
  }
  return true;
}

bool Levels::Delete(int level) {
  auto it = Find(level);
  if (it != levels_.end()) {
    levels_.erase(it);
    WriteToConfig();
    return true;
  }

  return false;
}

bool Levels::Edit(int level, std::string const &name,
                  std::string const &commands, std::string const &greeting,
                  int updated) {
  auto it = FindConst(level);
  if (it == levels_.end()) {
    return Add(level, name, commands, greeting);
  }

  const auto CMDS_UPDATED = 1;
  const auto GREETING_UPDATED = 2;
  const auto NAME_UPDATED = 4;

  if (updated & CMDS_UPDATED) {
    it->get()->commands = commands;
  }

  if (updated & GREETING_UPDATED) {
    it->get()->greeting = greeting;
  }

  if (updated & NAME_UPDATED) {
    it->get()->name = name;
  }

  if (!WriteToConfig()) {
    return false;
  }
  return true;
}

bool Levels::CreateDefaultLevels() {
  levels_.clear();

  auto tempLevel = std::make_shared<Level>(
      0, "Visitor", "Welcome Visitor [n]^7! Your last visit was on [t]!", "a");
  levels_.push_back(tempLevel);

  tempLevel = std::make_shared<Level>(
      1, "Friend", "Welcome Friend [n]^7! Your last visit was [d] ago!", "a");
  levels_.push_back(tempLevel);

  tempLevel = std::make_shared<Level>(2, "Moderator",
                                      "Welcome Moderator [n]^7!", "*-Asv");
  levels_.push_back(tempLevel);

  tempLevel = std::make_shared<Level>(3, "Administrator",
                                      "Welcome Administrator [n]^7!", "*");
  levels_.push_back(tempLevel);

  if (!WriteToConfig()) {
    return false;
  }
  return true;
}

void WriteString(const char *toWrite, fileHandle_t &f) {
  trap_FS_Write(toWrite, strlen(toWrite), f);
  trap_FS_Write("\n", 1, f);
}

void WriteInt(int toWrite, fileHandle_t &f) {
  const int BUFSIZE = 32;
  char buf[BUFSIZE];
  Com_sprintf(buf, sizeof(buf), "%d", toWrite);
  trap_FS_Write(buf, strlen(buf), f);
  trap_FS_Write("\n", 1, f);
}

bool Levels::SortByLevel(const std::shared_ptr<Level> &lhs,
                         const std::shared_ptr<Level> &rhs) {
  return lhs->level < rhs->level;
}

bool Levels::WriteToConfig() {
  auto f = 0;
  trap_FS_FOpenFile(g_levelConfig.string, &f, FS_WRITE);

  sort(levels_.begin(), levels_.end(), SortByLevel);
  for (ConstIter it = levels_.begin(); it != levels_.end(); ++it) {
    trap_FS_Write("[level]\n", 8, f);
    trap_FS_Write("level = ", 8, f);
    WriteInt(it->get()->level, f);

    trap_FS_Write("name = ", 7, f);
    WriteString(it->get()->name.c_str(), f);

    trap_FS_Write("cmds = ", 7, f);
    WriteString(it->get()->commands.c_str(), f);

    trap_FS_Write("greeting = ", 11, f);
    WriteString(it->get()->greeting.c_str(), f);
    trap_FS_Write("\n", 1, f);
  }

  trap_FS_FCloseFile(f);
  return true;
}

Levels::Level const *Levels::GetLevel(int level) {
  auto it = levels_.begin();
  auto end = levels_.end();
  for (; it != end; ++it) {
    if (it->get()->level == level) {
      return it->get();
    }
  }

  return dummyLevel_.get();
}

void Levels::PrintLevelInfo(gentity_t *ent) {
  Printer::chat(ent, "^3levelinfo: ^7check console for more information.");
  std::string msg = "Levels:";

  for (const auto &lvl : levels_) {
    msg += ETJump::stringFormat(" %i,", lvl->level);
  }

  if (!msg.empty() && msg.back() == ',') {
    msg.replace(msg.length() - 1, 1, "\n");
  }

  Printer::console(ent, msg);
}

void Levels::PrintLevelInfo(gentity_t *ent, int level) {
  for (const auto &lvl : levels_) {
    if (lvl->level == level) {
      Printer::chat(ent, "^3levelinfo: ^7check console for more information.");
      Printer::console(ent, ETJump::stringFormat("^5Level: ^7%d\n"
                                                 "^5Name: ^7%s\n"
                                                 "^5Commands: ^7%s\n"
                                                 "^5Greeting: ^7%s",
                                                 level, lvl->name,
                                                 lvl->commands, lvl->greeting));
      return;
    }
  }

  Printer::chat(ent,
                "^3levelinfo: ^7undefined level: " + std::to_string(level));
}

bool Levels::LevelExists(int level) const {
  auto it = levels_.begin();

  for (; it != levels_.end(); ++it) {
    if (it->get()->level == level) {
      return true;
    }
  }
  return false;
}

void Levels::PrintLevels() {
  auto it = levels_.begin();

  std::string level;

  for (; it != levels_.end(); ++it) {
    level += std::to_string(it->get()->level) + "\n" + it->get()->name + "\n" +
             it->get()->commands + "\n" + it->get()->greeting + "\n";

    G_LogPrintf(level.c_str());

    level = "";
  }
}

std::string Levels::ErrorMessage() const { return errorMessage; }

Levels::ConstIter Levels::FindConst(int level) {
  ConstIter it = levels_.begin();
  for (; it != levels_.end(); ++it) {
    if (it->get()->level == level) {
      return it;
    }
  }
  return it;
}

Levels::Iter Levels::Find(int level) {
  auto it = levels_.begin();
  for (; it != levels_.end(); ++it) {
    if (it->get()->level == level) {
      return it;
    }
  }
  return it;
}

int ReadInt(const char **configFile) {
  auto token = COM_ParseExt(configFile, qfalse);

  if (!Q_stricmp(token, "=")) {
    token = COM_ParseExt(configFile, qfalse);
  } else {
    G_LogPrintf("readconfig: missing = before \"%s\" on line %d.", token,
                COM_GetCurrentParseLine());
  }
  return Q_atoi(token);
}

std::string ReadString(const char **configFile) {
  auto token = COM_ParseExt(configFile, qfalse);

  if (!Q_stricmp(token, "=")) {
    token = COM_ParseExt(configFile, qfalse);
  } else {
    G_LogPrintf("readconfig: missing = before \"%s\" on line %d.", token,
                COM_GetCurrentParseLine());
  }

  std::string output;

  while (token[0]) {
    output += token;
    output.push_back(' ');
    token = COM_ParseExt(configFile, qfalse);
  }

  return ETJump::trimEnd(std::move(output));
}

bool Levels::ReadFromConfig() {
  auto f = 0;
  auto len = trap_FS_FOpenFile(g_levelConfig.string, &f, FS_READ);
  if (len < 0) {
    CreateDefaultLevels();
    return true;
  }
  std::unique_ptr<char[]> file;
  try {
    file = std::unique_ptr<char[]>(new char[len + 1]);
  } catch (...) {
    G_Error("Failed to allocate memory to parse level config.");
    trap_FS_FCloseFile(f);
    return false;
  }

  trap_FS_Read(file.get(), len, f);
  file[len] = 0;
  trap_FS_FCloseFile(f);

  const char *token = nullptr;
  auto levelOpen = false;
  std::shared_ptr<Level> tempLevel;

  levels_.clear();

  const char *file2 = file.get();

  token = COM_Parse(&file2);

  while (*token) {
    if (!Q_stricmp(token, "[level]")) {
      if (levelOpen) {
        levels_.push_back(tempLevel);
      }
      levelOpen = false;
    } else if (!Q_stricmp(token, "cmds")) {
      tempLevel->commands = ReadString(&file2);
    } else if (!Q_stricmp(token, "level")) {
      tempLevel->level = ReadInt(&file2);
    } else if (!Q_stricmp(token, "greeting")) {
      tempLevel->greeting = ReadString(&file2);
    } else if (!Q_stricmp(token, "name")) {
      tempLevel->name = ReadString(&file2);
    } else {
      G_LogPrintf("readconfig: parse error near %s on line %d", token,
                  COM_GetCurrentParseLine());
    }

    if (!Q_stricmp(token, "[level]")) {
      try {
        tempLevel = std::make_shared<Level>(0, "", "", "");
      } catch (...) {
        G_Error("Failed to allocate memory for "
                "a level.");
        return false;
      }

      levelOpen = true;
    }

    token = COM_Parse(&file2);
  }

  if (levelOpen) {
    levels_.push_back(tempLevel);
  }
  return true;
}
