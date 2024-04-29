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

#include "g_local.h"
#include "etj_json_utilities.h"
#include "etj_chat_replay.h"
#include "etj_string_utilities.h"
#include "etj_log.h"

namespace ETJump {
Log logger("chatreplay");

ChatReplay::ChatReplay() {
  chatReplayBuffer.clear();
  readChatsFromFile();
}

ChatReplay::~ChatReplay() { writeChatsToFile(); }

void ChatReplay::storeChatMessage(int clientNum, const std::string &name,
                                  const std::string &message, bool localize,
                                  bool encoded) {
  ChatMessage msg{};

  msg.clientNum = clientNum;
  msg.name = name;
  msg.localize = localize;
  msg.encoded = encoded;
  msg.message = message;

  chatReplayBuffer.emplace_back(msg);

  if (chatReplayBuffer.size() > MAX_CHAT_REPLAY_BUFFER) {
    chatReplayBuffer.pop_front();
  }
}

void ChatReplay::sendChatMessages(gentity_t *ent) {
  if (!ent || !ent->client) {
    return;
  }

  for (const auto &msg : chatReplayBuffer) {
    // skip messages from ignored clients
    if (COM_BitCheck(ent->client->sess.ignoreClients, msg.clientNum)) {
      continue;
    }

    const std::string &message = parseChatMessage(msg);
    trap_SendServerCommand(ClientNum(ent), message.c_str());
  }
}

std::string ChatReplay::parseChatMessage(const ChatReplay::ChatMessage &msg) {
  const char *cmd = msg.encoded ? "enc_chat" : "chat";
  return stringFormat("%s \"%s%c%c%s\" %i %i", cmd, msg.name, Q_COLOR_ESCAPE,
                      COLOR_GREEN, msg.message, msg.clientNum, msg.localize);
}

void ChatReplay::readChatsFromFile() {
  Json::Value root;

  if (!JsonUtils::readFile(chatReplayFile, root)) {
    logger.error("Couldn't open chat replay file '%s' for reading.",
                 chatReplayFile);
    return;
  }

  for (const auto &msg : root) {
    const int clientNum = msg["clientnum"].asInt();
    const std::string &name = msg["name"].asString();
    const std::string &message = msg["message"].asString();
    const bool localize = msg["localize"].asBool();
    const bool encoded = msg["encoded"].asBool();

    storeChatMessage(clientNum, name, message, localize, encoded);
  }
}

void ChatReplay::writeChatsToFile() {
  Json::Value root;
  Json::Value chat;

  for (const auto &msg : chatReplayBuffer) {
    chat["clientNum"] = msg.clientNum;
    chat["name"] = msg.name;
    chat["message"] = msg.message;
    chat["localize"] = msg.localize;
    chat["encoded"] = msg.encoded;

    root.append(chat);
  }

  if (!ETJump::JsonUtils::writeFile(chatReplayFile, root)) {
    logger.error("Could not open chat replay file '%s' for writing.",
                 chatReplayFile);
  }
}
} // namespace ETJump
