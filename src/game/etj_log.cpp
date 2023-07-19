#include <string>

#include "etj_log.h"
#include "etj_printer.h"

namespace ETJump {
void Log::println(const std::string &level,
                  const std::string &message) const {
  Printer::LogPrintln(_name + " [" + level + "]: " + message);
}
}


