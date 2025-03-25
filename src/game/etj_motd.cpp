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

#include "etj_motd.h"
#include "json/json.h"
#include "etj_local.h"
#include "utilities.hpp"
#include "etj_filesystem.h"
#include "etj_printer.h"
#include <fstream>
#include "etj_json_utilities.h"

namespace ETJump {
Motd::Motd(std::unique_ptr<Log> log)
    : initialized(false), logger(std::move(log)) {}

void Motd::initialize() {
  if (strlen(g_motdFile.string) == 0) {
    return;
  }

  if (!FileSystem::exists(g_motdFile.string)) {
    logger->info("MOTD file '%s' was defined but could not be found. To "
                 "generate a sample file, use '/generateMotd'.",
                 g_motdFile.string);
    return;
  }

  Json::Value root;

  if (!JsonUtils::readFile(g_motdFile.string, root, &errors)) {
    logger->error(errors);
    return;
  }

  if (!JsonUtils::parseValue(chatMotd, root["chat_message"], &errors,
                             "chat_message")) {
    logger->error(errors);
  }

  if (!JsonUtils::parseValue(consoleMotd, root["console_message"], &errors,
                             "console_message")) {
    logger->error(errors);
  }

  if (errors.empty()) {
    logger->info("Successfully initialized MOTD system.");
    initialized = true;
  } else {
    logger->error("Missing information from MOTD file '%s'. Make sure both "
                  "'chat_message' and 'console_message' fields are defined.",
                  g_motdFile.string);
  }
}

void Motd::printMotd(gentity_t *ent) const {
  if (!initialized) {
    return;
  }

  Printer::chat(ent, chatMotd);
  Printer::console(ent, consoleMotd + "\n");
}

void Motd::generateMotdFile() {
  const Arguments argv = GetArgs();

  const bool overwrite = argv->size() > 1 && argv->at(1) == "-f";

  if (argv->size() == 1 || !overwrite) {
    if (FileSystem::exists(g_motdFile.string)) {
      G_Printf("A MOTD file '%s' already exists. Use '/generateMotd -f' to "
               "overwrite the existing file.\n",
               g_motdFile.string);
      return;
    }
  }

  if (overwrite) {
    G_Printf("Overwriting MOTD file '%s' with a default one.\n",
             g_motdFile.string);
  }

  Json::Value root;
  root["chat_message"] = "This is the chat message.";
  root["console_message"] = "This is the console message.";

  if (JsonUtils::writeFile(g_motdFile.string, root, &errors)) {
    G_Printf("Generated new motd file '%s'\n", g_motdFile.string);
    initialize();
  } else {
    G_Printf("%s", errors.c_str());
  }
}
} // namespace ETJump
