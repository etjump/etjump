#include <string>

#include "etj_log.h"
#include "etj_printer.h"

namespace ETJump {
std::mutex Log::_messagesLock;
std::vector<std::string> Log::_messages;

void Log::println(const std::string &level, const std::string &message) const {
  std::lock_guard<std::mutex> lock(Log::_messagesLock);
  _messages.push_back(stringFormat("%s [%s]: %s", _name, level, message));
}

void Log::processMessages() {
  std::lock_guard<std::mutex> lock(Log::_messagesLock);
  for (const auto &msg : _messages) {
    Printer::LogPrintln(msg);
  }
  _messages.clear();
}
}
