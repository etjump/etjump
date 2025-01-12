#include <vector>

#include "utilities.hpp"
#include "etj_local.h"
#include "etj_string_utilities.h"

using std::string;
using std::vector;

/*
 * Argument handling
 */

Arguments GetArgs() {
  int argc = trap_Argc();
  static vector<string> argv;
  argv.clear();

  for (int i = 0; i < argc; i++) {
    char arg[MAX_TOKEN_CHARS];
    trap_Argv(i, arg, sizeof(arg));
    argv.push_back(arg);
  }
  return &argv;
}

Arguments GetSayArgs(int start /*= 0*/) {
  int argc = Q_SayArgc();

  static vector<string> argv;
  argv.clear();

  if (start >= argc) {
    return &argv;
  }

  for (int i = start; i < argc; i++) {
    char arg[MAX_TOKEN_CHARS];
    Q_SayArgv(i, arg, sizeof(arg));
    argv.push_back(arg);
  }
  return &argv;
}

/*
 * Conversions
 */

bool ToInt(const string &toConvert, int &value) {
  try {
    value = std::stoi(toConvert);
  } catch (...) {
    return false;
  }
  return true;
}

bool ToUnsigned(const std::string &toConvert, unsigned &value) {
  try {
    value = std::stoul(toConvert);
  } catch (...) {
    return false;
  }
  return true;
}

bool ToFloat(const std::string &toConvert, float &value) {
  try {
    value = std::stof(toConvert);
  } catch (...) {
    return false;
  }
  return true;
}

std::string ToString(vec3_t toConvert) {
  string vec3 = "(" + std::to_string(toConvert[0]) + ", " +
                std::to_string(toConvert[1]) + ", " +
                std::to_string(toConvert[2]) + ")";
  return vec3;
}

std::string ToString(vec_t x, vec_t y, vec_t z) {
  string vec3 = "(" + std::to_string(x) + ", " + std::to_string(y) + ", " +
                std::to_string(z) + ")";
  return vec3;
}

gentity_t *PlayerGentityFromString(char *name, char *err, int size,
                                   team_t filter) {
  int pids[MAX_CLIENTS];
  gentity_t *player;

  if (ClientNumbersFromString(name, pids) != 1) {
    if (!G_MatchOnePlayer(pids, err, size, filter)) {
      return nullptr;
    }
  }

  player = g_entities + pids[0];
  return player;
}

gentity_t *PlayerGentityFromString(const std::string &name, std::string &err,
                                   team_t filter) {
  char errorMsg[MAX_TOKEN_CHARS] = "\0";
  int pids[MAX_CLIENTS];
  gentity_t *player;

  if (ClientNumbersFromString(name.c_str(), pids) != 1) {
    if (!G_MatchOnePlayer(pids, errorMsg, sizeof(errorMsg), filter)) {
      err = errorMsg;
      return nullptr;
    }
  }

  player = g_entities + pids[0];
  return player;
}

std::string SayArgv(int n) {
  if (n >= Q_SayArgc()) {
    return "";
  } else {
    char arg[MAX_TOKEN_CHARS] = "\0";
    Q_SayArgv(n, arg, sizeof(arg));

    return arg;
  }
}

string ValueForKey(gentity_t *ent, const std::string &key) {
  char userinfo[MAX_INFO_STRING] = "\0";
  char *value = NULL;

  trap_GetUserinfo(ClientNum(ent), userinfo, sizeof(userinfo));
  value = Info_ValueForKey(userinfo, key.c_str());
  if (!value) {
    return "";
  }

  return value;
}

string ValueForKey(int clientNum, const std::string &key) {
  char userinfo[MAX_INFO_STRING] = "\0";
  char *value = NULL;

  trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));
  value = Info_ValueForKey(userinfo, key.c_str());
  if (!value) {
    return "";
  }

  return value;
}

string TimeStampToString(int t) {
  char buf[MAX_TOKEN_CHARS];
  struct tm *lt = NULL;
  time_t toConvert = t;
  lt = localtime(&toConvert);
  if (t > 0) {
    // day / month / year
    strftime(buf, sizeof(buf), "%d/%m/%y %H:%M:%S", lt);
  } else {
    return "never";
  }

  return std::string(buf);
}

std::string TimeStampDifferenceToString(int diff) {
  constexpr int MINUTE = 60;
  constexpr int HOUR = 60 * MINUTE;
  constexpr int DAY = 24 * HOUR;
  constexpr int WEEK = 7 * DAY;
  constexpr int MONTH = 30 * DAY;
  constexpr int YEAR = 365 * DAY;

  if (diff < HOUR) {
    return ETJump::getMinutesString(diff / MINUTE);
  } else if (diff < DAY) {
    return ETJump::getHoursString(diff / HOUR);
  } else if (diff < WEEK) {
    return ETJump::getDaysString(diff / DAY);
  } else if (diff < MONTH) {
    return ETJump::getWeeksString(diff / WEEK);
  } else if (diff < YEAR) {
    return ETJump::getMonthsString(diff / MONTH);
  } else {
    return ETJump::getYearsString(diff / YEAR);
  }
}

bool ValidGuid(std::string guid) {
  const unsigned GUID_SIZE = 40;
  if (guid.size() != GUID_SIZE) {
    return false;
  }

  for (size_t i = 0; i < guid.size(); i++) {
    if (guid[i] < '0' || guid[i] > 'F') {
      return false;
    }
  }
  return true;
}

const char *EscapeString(const char *in) {
  string str = in;
  ETJump::StringUtil::replaceAll(str, "=", "\x19=");
  static char out[MAX_TOKEN_CHARS] = "\0";
  Q_strncpyz(out, str.c_str(), sizeof(out));
  return out;
}

std::vector<std::string> getNames(const std::vector<int> &ids) {
  std::vector<std::string> names;

  for (auto &id : ids) {
    std::string name = (g_entities + id)->client->pers.netname;

    // escape '=' for QP-encoding
    ETJump::StringUtil::replaceAll(name, "=", "\x19=");
    names.push_back(name);
  }

  return names;
}

std::vector<int> getMatchingIds(const std::string &name) {
  int pids[MAX_CLIENTS];
  auto found = ClientNumbersFromString(name.c_str(), pids);
  std::vector<int> pidsVector;
  if (found) {
    for (auto i = 0; pids[i] != -1; i++) {
      pidsVector.push_back(pids[i]);
    }
  }
  return pidsVector;
}

std::string interpolateNametags(std::string input, const int color) {
  std::string interpolated;
  const std::vector<std::string> split = ETJump::StringUtil::split(input, "@");

  if (split.size() == 1 || split.size() == 2) {
    return input;
  }

  int i = 1;
  int len;
  std::string colorCode = "^";
  colorCode += static_cast<char>(color);

  for (len = static_cast<int>(split.size()); i < len; i += 2) {
    interpolated += split[i - 1];

    if (split[i].empty()) {
      interpolated += "@";
    } else {
      auto names = getNames(getMatchingIds(split[i]));
      if (!names.empty()) {
        const std::string &splitStr = colorCode + ", ^7";
        interpolated +=
            "^7" + ETJump::StringUtil::join(names, splitStr) + colorCode;
      } else {
        interpolated += "@" + split[i] + "@";
      }
    }
  }

  if (len % 2 != 0 || i == len) {
    interpolated += split[len - 1];
  }

  return interpolated;
}

const char *interpolateNametags(const char *text, const int color) {
  static char buf[MAX_SAY_TEXT] = "\0";

  auto interpolated = interpolateNametags(std::string(text), color);

  Q_strncpyz(buf, interpolated.c_str(), sizeof(buf));
  return buf;
}

const char *findAndReplaceNametags(const char *text, const char *name) {
  static char buf[MAX_SAY_TEXT] = "\0";

  auto str = std::string(text);
  ETJump::StringUtil::replaceAll(str, "[n]", name);

  Q_strncpyz(buf, str.c_str(), sizeof(buf));
  return buf;
}
