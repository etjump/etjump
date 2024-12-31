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

#include "g_local.h"
#include "etj_json_utilities.h"
#include "etj_chat_replay.h"
#include "etj_string_utilities.h"
#include "etj_time_utilities.h"
#include "etj_session.h"

namespace ETJump {

ChatReplay::ChatReplay(std::unique_ptr<Log> log) : logger(std::move(log)) {
  chatReplayBuffer.clear();
  readChatsFromFile();
}

void ChatReplay::createChatMessage(const int clientNum, const std::string &name,
                                   const std::string &message,
                                   const bool localize, const bool encoded) {
  ChatMessage msg{};

  msg.clientNum = clientNum;
  msg.name = name;
  msg.localize = localize;
  msg.encoded = encoded;
  msg.message = sanitize(message);
  msg.expired = false;

  time_t t;
  t = std::time(&t);

  // FIXME: 32-bit time
  msg.timestamp = static_cast<int>(t);

  storeChatMessage(msg);
}

void ChatReplay::storeChatMessage(const ChatMessage &msg) {
  chatReplayBuffer.emplace_back(msg);

  if (chatReplayBuffer.size() > MAX_CHAT_REPLAY_BUFFER) {
    chatReplayBuffer.pop_front();
  }
}

void ChatReplay::sendChatMessages(gentity_t *ent) {
  if (!ent || !ent->client) {
    return;
  }

  if (!g_chatReplay.integer) {
    return;
  }

  // shouldn't ever happen but just in case,
  // so we don't print out the info when there's no messages
  if (chatReplayBuffer.empty()) {
    return;
  }

  const int clientNum = ClientNum(ent);

  // if messages are set to expire, mark any chats that are too old
  if (g_chatReplayMaxMessageAge.integer > 0) {
    int sessStartTime = session->getSessionStartTime(clientNum);

    // FIXME: 32-bit time
    sessStartTime -= g_chatReplayMaxMessageAge.integer * 60;
    const Time maxAge = Time::fromInt(sessStartTime);

    bool allExpired = true;

    for (auto &msg : chatReplayBuffer) {
      if (maxAge > Time::fromInt(msg.timestamp)) {
        msg.expired = true;
      } else {
        msg.expired = false;
        allExpired = false;
      }
    }

    // no valid messages to send
    if (allExpired) {
      return;
    }
  }

  // send this with raw trap_SendServerCommand instead of Printer,
  // so we can omit team flags easily on client side
  // timestamp is set to -1 to identify this from other chat replays
  trap_SendServerCommand(
      clientNum,
      "chat \"^gServer: replaying latest chat messages:\" -1 0 1 -1");

  for (const auto &msg : chatReplayBuffer) {
    // skip messages from ignored clients
    if (COM_BitCheck(ent->client->sess.ignoreClients, msg.clientNum)) {
      continue;
    }

    if (msg.expired) {
      continue;
    }

    const std::string &message = parseChatMessage(msg);
    trap_SendServerCommand(clientNum, message.c_str());
  }
}

std::string ChatReplay::parseChatMessage(const ChatMessage &msg) {
  const char *cmd = msg.encoded ? "enc_chat" : "chat";
  return stringFormat("%s \"^7%s%c%c%s\" %i %i 1 %i", cmd, msg.name,
                      Q_COLOR_ESCAPE, COLOR_LTGREY, msg.message, msg.clientNum,
                      msg.localize, msg.timestamp);
}

void ChatReplay::readChatsFromFile() {
  Json::Value root;

  if (!JsonUtils::readFile(chatReplayFile, root, &errors)) {
    logger->error(errors);
    return;
  }

  for (const auto &message : root) {
    ChatMessage msg{};

    if (JsonUtils::parseValue(msg.clientNum, message["clientNum"], &errors,
                              "clientNum") &&
        JsonUtils::parseValue(msg.name, message["name"], &errors, "name") &&
        JsonUtils::parseValue(msg.message, message["message"], &errors,
                              "message") &&
        JsonUtils::parseValue(msg.localize, message["localize"], &errors,
                              "localize") &&
        JsonUtils::parseValue(msg.encoded, message["encoded"], &errors,
                              "encoded") &&
        JsonUtils::parseValue(msg.timestamp, message["timestamp"], &errors,
                              "timestamp")) {
      storeChatMessage(msg);
    } else {
      logger->error(errors);
    }
  }
}

void ChatReplay::writeChatsToFile() {
  Json::Value root;

  for (const auto &msg : chatReplayBuffer) {
    Json::Value chat;

    chat["clientNum"] = msg.clientNum;
    chat["name"] = msg.name;
    chat["message"] = msg.message;
    chat["localize"] = msg.localize;
    chat["encoded"] = msg.encoded;
    chat["timestamp"] = msg.timestamp;

    root.append(chat);
  }

  if (!JsonUtils::writeFile(chatReplayFile, root, &errors)) {
    logger->error(errors);
  }
}
} // namespace ETJump
