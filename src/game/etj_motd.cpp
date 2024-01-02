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

#include "etj_motd.h"
#include "json/json.h"
#include "etj_local.h"
#include "utilities.hpp"
#include <fstream>

Motd::Motd() : initialized_(false) {}

Motd::~Motd() {}

void Motd::Initialize() {
  if (strlen(g_motdFile.string) == 0) {
    return;
  }

  std::string filePath = GetPath(g_motdFile.string);
  std::ifstream f(filePath.c_str());

  if (!f) {
    G_LogPrintf("MOTD: g_motdFile was defined but couldn't "
                "find the motd file. To "
                "generate an example file do /rcon "
                "generateMotd <file name>.\n");
    return;
  }

  std::string content((std::istreambuf_iterator<char>(f)),
                      std::istreambuf_iterator<char>());

  Json::Value root;
  Json::Reader reader;

  if (!reader.parse(content, root)) {
    G_LogPrintf("MOTD: failed to parse motd file: %s\n",
                reader.getFormattedErrorMessages().c_str());
    return;
  }

  try {
    chatMessage_ = root["chat_message"].asString();
    motd_ = root["console_message"].asString();
  } catch (...) {
    G_LogPrintf("MOTD: missing information from motd file. Are you "
                "sure both "
                "chat_message and console_message are defined?\n");
  }

  G_LogPrintf("MOTD: initialized.\n");

  initialized_ = true;
}

void Motd::PrintMotd(gentity_t *ent) {
  if (initialized_) {
    ChatPrintTo(ent, chatMessage_);
    ConsolePrintTo(ent, motd_);
  }
}

void Motd::GenerateMotdFile() {
  Arguments argv = GetArgs();
  std::ifstream in(GetPath(g_motdFile.string).c_str());
  if (in.good()) {
    if (argv->size() == 1) {
      G_Printf("A motd file exists. Are you sure you want "
               "to overwrite the "
               "file? If so, do /rcon generatemotd -f.\n");
      return;
    }
    if (argv->size() == 2 && argv->at(1) == "-f") {
      G_LogPrintf("Overwriting motd file with a "
                  "default one.\n");
    } else {
      G_Printf("Unknown argument \"%s\".\n", argv->at(1).c_str());
      return;
    }
  }

  Json::Value root;
  root["chat_message"] = "This is the chat message.";
  root["console_message"] = "This is the console message.";
  Json::StyledWriter writer;
  std::string output = writer.write(root);
  std::ofstream fOut(GetPath(g_motdFile.string).c_str());
  if (!fOut) {
    G_Printf("Couldn't open file \"%s\" defined in g_motdFile.\n",
             g_motdFile.string);
    return;
  }
  fOut << output;
  fOut.close();
  G_Printf("Generated new motd file \"%s\"\n", g_motdFile.string);
  Initialize();
}
