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

#include <sqlite3.h>
#include <ctime>
#include <thread>

#include "etj_timerun.h"
#include "etj_sqlite_wrapper.h"
#include "etj_printer.h"
#include "etj_utilities.h"
#include "etj_string_utilities.h"
#include "etj_time_utilities.h"
#include "g_local.h"

std::string millisToString(int millis) {
  int minutes, seconds;

  minutes = millis / 60000;
  millis -= minutes * 60000;
  seconds = millis / 1000;
  millis -= seconds * 1000;

  return ETJump::stringFormat("%02d:%02d.%03d", minutes, seconds, millis);
}

std::string dateToFormat(int date) {
  char buffer[128] = "\0";
  auto t = static_cast<time_t>(date);
  strftime(buffer, sizeof(buffer), "%d.%m.%Y", localtime(&t));
  return buffer;
}

std::string GetColumnText(sqlite3_stmt *stmt, int index) {
  auto text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, index));
  return text ? text : "";
}

bool Timerun::init(const std::string &database, const std::string &currentMap) {
  SQLiteWrapper wrapper;

  Printer::LogPrintln("Opening timeruns database: " + database);

  _recordsByName.clear();
  for (auto &p : _players) {
    p = nullptr;
  }
  _currentMap = currentMap;
  _database = database;
  for (auto &s : _sorted) {
    s.second = false;
  }

  if (!wrapper.open(database.c_str())) {
    _message =
        ETJump::stringFormat("Timerun::init: couldn't open database. error "
                             "code: %d. error message: %s.",
                             wrapper.errorCode(), wrapper.errorMessage());
    return false;
  }

  if (!wrapper.prepare("CREATE TABLE IF NOT EXISTS records (id INTEGER PRIMARY "
                       "KEY AUTOINCREMENT, "
                       "time INT NOT NULL, "
                       "record_date INT NOT NULL, "
                       "map TEXT NOT NULL, "
                       "run TEXT NOT NULL, "
                       "user_id INT NOT NULL, "
                       "player_name TEXT NOT NULL);")) {
    _message =
        ETJump::stringFormat("Timerun::init: couldn't prepare create table "
                             "statement. error code: %d. error message: %s.",
                             wrapper.errorCode(), wrapper.errorMessage());
    return false;
  }

  if (!wrapper.execute()) {
    _message =
        ETJump::stringFormat("Timerun::init: couldn't execute statement. error "
                             "code: %d. error message: %s.",
                             wrapper.errorCode(), wrapper.errorMessage());
    return false;
  }

  if (!wrapper.prepare("SELECT id, time, run, user_id, player_name, "
                       "record_date FROM records WHERE map=?;")) {
    _message =
        ETJump::stringFormat("Timerun::init: couldn't prepare select runs "
                             "statement. error code: %d. error message: %s.",
                             wrapper.errorCode(), wrapper.errorMessage());
    return false;
  }

  if (!wrapper.bindText(1, currentMap)) {
    _message =
        ETJump::stringFormat("Timerun::init: couldn't bind current map to "
                             "statement. error code: %d. error message: %s.",
                             wrapper.errorCode(), wrapper.errorMessage());
    return false;
  }

  auto stmt = wrapper.getStatement();
  auto rc = 0;
  for (rc = sqlite3_step(stmt); rc == SQLITE_ROW; rc = sqlite3_step(stmt)) {
    Record *record = new Record();

    record->id = sqlite3_column_int(stmt, 0);
    record->time = sqlite3_column_int(stmt, 1);
    record->run = GetColumnText(stmt, 2);
    record->userId = sqlite3_column_int(stmt, 3);
    record->playerName = GetColumnText(stmt, 4);
    record->date = sqlite3_column_int(stmt, 5);
    record->map = currentMap;

    _recordsByName[record->run].push_back(std::unique_ptr<Record>(record));
  }
  if (rc != SQLITE_DONE) {
    _message =
        ETJump::stringFormat("Timerun::init: couldn't bind current map to "
                             "statement. error code: %d. error message: %s",
                             rc, wrapper.getSQLiteErrorMessage());
    return false;
  }

  Printer::LogPrint(ETJump::stringFormat(
      "Successfully loaded %d records from database\n", _recordsByName.size()));

  return true;
}

void Timerun::startTimer(const std::string &runName, int clientNum,
                         const std::string &currentName, int raceStartTime) {
  auto player = _players[clientNum].get();

  if (player == nullptr) {
    return;
  }

  if (player->racing) {
    return;
  }

  player->racing = true;
  player->name = currentName;
  player->runStartTime = raceStartTime;
  player->completionTime = 0;
  player->currentRunName = runName;
  startNotify(clientNum);

  Utilities::startRun(clientNum);
}

void Timerun::connectNotify(int clientNum) {
  for (int idx = 0; idx < 64; ++idx) {
    auto player = _players[idx].get();
    if (player && player->currentRunName.length() > 0) {
      auto previousRecord = findPreviousRecord(player);

      int fastestCompletionTime = -1;
      if (previousRecord) {
        fastestCompletionTime = previousRecord->time;
      }
      Printer::SendCommand(clientNum,
                           ETJump::stringFormat("timerun start %d %d \"%s\" %d",
                                                idx, player->runStartTime,
                                                player->currentRunName,
                                                fastestCompletionTime));
    }
  }
}

void Timerun::startNotify(int clientNum) {
  auto player = _players[clientNum].get();
  auto spectators = Utilities::getSpectators(clientNum);
  auto previousRecord = findPreviousRecord(player);

  int fastestCompletionTime = -1;
  if (previousRecord) {
    fastestCompletionTime = previousRecord->time;
  }
  Printer::SendCommand(
      clientNum,
      ETJump::stringFormat("timerun_start %d \"%s\" %d", player->runStartTime,
                           player->currentRunName, fastestCompletionTime));
  Printer::SendCommand(
      spectators,
      ETJump::stringFormat("timerun_start_spec %d %d \"%s\" %d", clientNum,
                           player->runStartTime, player->currentRunName,
                           fastestCompletionTime));
  Printer::SendCommandToAll(ETJump::stringFormat(
      "timerun start %d %d \"%s\" %d", clientNum, player->runStartTime,
      player->currentRunName, fastestCompletionTime));
}

bool Timerun::isDebugging(int clientNum) {
  std::vector<std::string> debuggers;

  if (g_debugTimeruns.integer > 0) {
    debuggers.push_back("Timerun");
  }

  if (g_debugTrackers.integer > 0) {
    debuggers.push_back("Tracker");
  }

  if (debuggers.size()) {
    Printer::SendPopupMessage(clientNum, "Record not saved:\n");

    for (auto &debugger : debuggers) {
      std::string fmt =
          ETJump::stringFormat("- ^3%s ^7debugging enabled.\n", debugger);
      Printer::SendPopupMessage(clientNum, fmt);
    }

    return true;
  }

  return false;
}

void Timerun::stopTimer(int clientNum, int commandTime, std::string runName) {
  Player *player = _players[clientNum].get();

  if (player == nullptr) {
    return;
  }

  if (player->racing && player->currentRunName == runName) {
    auto millis = commandTime - player->runStartTime;
    player->completionTime = millis;

    if (!g_cheats.integer && !isDebugging(clientNum)) {
      checkRecord(player, clientNum);
    }

    player->racing = false;

    Printer::SendCommand(clientNum,
                         ETJump::stringFormat("timerun_stop %d \"%s\"", millis,
                                              player->currentRunName));
    auto spectators = Utilities::getSpectators(clientNum);
    Printer::SendCommand(spectators,
                         ETJump::stringFormat("timerun_stop_spec %d %d \"%s\"",
                                              clientNum, millis,
                                              player->currentRunName));
    Printer::SendCommandToAll(ETJump::stringFormat("timerun stop %d %d \"%s\"",
                                                   clientNum, millis,
                                                   player->currentRunName));

    player->currentRunName = "";
    Utilities::stopRun(clientNum);
  }
}

std::string rankToString(int rank) {
  switch (rank) {
    case 1:
      return "^3#1";
    case 2:
      return "^z#2";
    case 3:
      return "^l#3";
    default:
      return "^7#" + std::to_string(rank);
  }
}

std::string diffToString(int selfTime, int otherTime) {
  auto diff = otherTime - selfTime;
  auto ams = std::abs(diff);
  auto diffComponents = ETJump::toClock(ams, false);

  const char *diffSign;
  if (diff > 0) {
    diffSign = "^2-";
  } else if (diff < 0) {
    diffSign = "^1+";
  } else {
    diffSign = "^7+";
  }

  return ETJump::stringFormat("%s%02i:%02i.%03i", diffSign, diffComponents.min,
                              diffComponents.sec, diffComponents.ms);
}

void Timerun::printRecordsForRun(int clientNum, const std::string &runName) {
  const std::pair<const std::basic_string<char>,
                  std::vector<std::unique_ptr<Record>>> *run = nullptr;

  for (const auto &iter : _recordsByName) {
    if (ETJump::sanitize(iter.first, true) == ETJump::sanitize(runName, true)) {
      run = &iter;
    }
  }

  if (run == nullptr) {
    Printer::SendConsoleMessage(
        clientNum, "^3error: ^7no records found by name: " + runName + "\n");
    return;
  }

  const auto self = _players[clientNum].get();
  auto foundSelf = false;

  std::string buffer =
      "^g=============================================================\n"
      " ^2Top 50 records for map: ^7" +
      _currentMap +
      "\n"
      "^g=============================================================\n";

  auto rank = 1;
  buffer += " ^2Run: ^7" + run->first + "\n\n";
  buffer += "^g Rank  Time       Difference     Player\n";

  // Get clients own time first for difference calculations
  int selfTime = 0;
  for (auto &record : run->second) {
    if (record->userId == self->userId) {
      selfTime = record->time;
      foundSelf = true;
      break;
    }
  }

  for (auto &record : run->second) {
    if (rank <= 50) {
      if (record->userId == self->userId) {
        buffer += ETJump::stringFormat(
            "^7%5s    ^7%s                 ^7%s ^7(^1You^7)\n",
            rankToString(rank), millisToString(record->time),
            record->playerName);
      } else {
        auto diff = foundSelf
                        ? diffToString(selfTime, record->time)
                        : "          "; // Just print bunch of whitespace as
                                        // difference if client has no record
        buffer += ETJump::stringFormat(
            "^7%5s    ^7%s  ^9%s     ^7%s\n", rankToString(rank),
            millisToString(record->time), diff, record->playerName);
      }
    } else {
      if (foundSelf) {
        break;
      }

      // FIXME: this is dead code as of now, fix the logic so this
      //  can actually get printed out when we have record outside top 50
      if (record->userId == self->userId) {
        buffer += ETJump::stringFormat(
            "^7%4s     ^7%s     ^7%s ^7(^1You^7)\n", rankToString(rank),
            millisToString(record->time), record->playerName);
        foundSelf = true;
      }
    }
    rank++;
  }

  if (!foundSelf) {
    buffer += "^7You haven't set a record on this run yet!\n";
  }

  buffer += "^g=============================================================\n";
  Printer::SendConsoleMessage(clientNum, buffer);
}

void Timerun::printCurrentMapRecords(int clientNum) {
  std::string buffer = "^g============================================="
                       "================\n"
                       " ^2Top records for map: ^7" +
                       _currentMap +
                       "\n"
                       "^g============================================="
                       "================\n";

  for (const auto &run : _recordsByName) {
    auto rank = 1;
    buffer += " ^2Run: ^7" + run.first + "\n\n";
    buffer += "^g Rank   Time        Player\n";
    for (const auto &record : run.second) {
      if (rank > 3) {
        break;
      }
      buffer += ETJump::stringFormat(
          "^7 %4s    ^7 %s   %s\n", rankToString(rank++),
          millisToString(record->time), record->playerName);
    }

    buffer += "^g=============================================="
              "===============\n";
  }

  Printer::SendConsoleMessage(clientNum, buffer);
}

void Timerun::interrupt(int clientNum) {
  Player *player = _players[clientNum].get();

  if (player == nullptr || !player->racing) {
    return;
  }

  player->racing = false;
  player->currentRunName = "";

  Utilities::stopRun(clientNum);
  Printer::SendCommand(clientNum, "timerun_interrupt");
  Printer::SendCommandToAll(
      ETJump::stringFormat("timerun interrupt %d", clientNum));
}

/**
 * Saves the record to database
 * @param update Inserts if this is set to false, else update
 * @param database The database file name
 * @param record The actual record
 */
static void SaveRecord(bool update, std::string database,
                       Timerun::Record record) {
  SQLiteWrapper wrapper;
  if (!wrapper.open(database)) {
    Printer::LogPrintln(
        ETJump::stringFormat("SaveRecord: Couldn't open database to save the "
                             "record. error code: %d. error message: %s.",
                             wrapper.errorCode(), wrapper.errorMessage()));
    return;
  }

  //    "CREATE TABLE IF NOT EXISTS records "
  //            "(id INTEGER PRIMARY KEY AUTOINCREMENT, "
  //            "time INT NOT NULL, "
  //            "record_date INT NOT NULL, "
  //            "map TEXT NOT NULL, "
  //            "run TEXT NOT NULL, "
  //            "user_id INT NOT NULL, "
  //            "player_name TEXT NOT NULL);"

  if (update) {
    if (record.id != -1) {
      if (!wrapper.prepare("UPDATE records SET time=?, "
                           "record_date=?, "
                           "player_name=? WHERE id=?;")) {
        Printer::LogPrintln(
            ETJump::stringFormat("SaveRecord::couldn't prepare "
                                 "update statement. error code: "
                                 "%d. "
                                 "error message: %s.",
                                 wrapper.errorCode(), wrapper.errorMessage()));
        return;
      }

      if (!wrapper.bindInteger(1, record.time)) {
        Printer::LogPrintln(ETJump::stringFormat("SaveRecord::couldn't bind "
                                                 "time to update statement. "
                                                 "error code: "
                                                 "%d. error message: %s.",
                                                 wrapper.errorCode(),
                                                 wrapper.errorMessage()));
        return;
      }

      if (!wrapper.bindInteger(2, record.date)) {
        Printer::LogPrintln(ETJump::stringFormat("SaveRecord::couldn't bind "
                                                 "date to update statement. "
                                                 "error code: "
                                                 "%d. error message: %s.",
                                                 wrapper.errorCode(),
                                                 wrapper.errorMessage()));
        return;
      }

      if (!wrapper.bindText(3, record.playerName)) {
        Printer::LogPrintln(ETJump::stringFormat("SaveRecord::couldn't bind "
                                                 "name to update statement. "
                                                 "error code: "
                                                 "%d. error message: %s.",
                                                 wrapper.errorCode(),
                                                 wrapper.errorMessage()));
        return;
      }

      if (!wrapper.bindInteger(4, record.id)) {
        Printer::LogPrintln(ETJump::stringFormat("SaveRecord::couldn't bind id "
                                                 "to update statement. error "
                                                 "code: %d. "
                                                 "error message: %s.",
                                                 wrapper.errorCode(),
                                                 wrapper.errorMessage()));
        return;
      }
    } else {
      if (!wrapper.prepare("UPDATE records SET time=?, "
                           "record_date=?, player_name=? WHERE "
                           "map=? AND run=? and user_id=?;")) {
        Printer::LogPrintln(
            ETJump::stringFormat("SaveRecord::couldn't prepare "
                                 "update statement. error code: "
                                 "%d. "
                                 "error message: %s.",
                                 wrapper.errorCode(), wrapper.errorMessage()));
        return;
      }

      if (!wrapper.bindInteger(1, record.time)) {
        Printer::LogPrintln(ETJump::stringFormat("SaveRecord::couldn't bind "
                                                 "time to update statement. "
                                                 "error code: "
                                                 "%d. error message: %s.",
                                                 wrapper.errorCode(),
                                                 wrapper.errorMessage()));
        return;
      }

      if (!wrapper.bindInteger(2, record.date)) {
        Printer::LogPrintln(ETJump::stringFormat("SaveRecord::couldn't bind "
                                                 "date to update statement. "
                                                 "error code: "
                                                 "%d. error message: %s.",
                                                 wrapper.errorCode(),
                                                 wrapper.errorMessage()));
        return;
      }

      if (!wrapper.bindText(3, record.playerName)) {
        Printer::LogPrintln(ETJump::stringFormat("SaveRecord::couldn't bind "
                                                 "name to update statement. "
                                                 "error code: "
                                                 "%d. error message: %s.",
                                                 wrapper.errorCode(),
                                                 wrapper.errorMessage()));
        return;
      }

      if (!wrapper.bindText(4, record.map)) {
        Printer::LogPrintln(
            ETJump::stringFormat("SaveRecord::couldn't bind map "
                                 "name to update statement. "
                                 "error "
                                 "code: %d. error message: %s.",
                                 wrapper.errorCode(), wrapper.errorMessage()));
        return;
      }

      if (!wrapper.bindText(5, record.run)) {
        Printer::LogPrintln(
            ETJump::stringFormat("SaveRecord::couldn't bind run "
                                 "name to update statement. "
                                 "error "
                                 "code: %d. error message: %s.",
                                 wrapper.errorCode(), wrapper.errorMessage()));
        return;
      }

      if (!wrapper.bindInteger(6, record.userId)) {
        Printer::LogPrintln(ETJump::stringFormat("SaveRecord::couldn't bind "
                                                 "user id to update statement. "
                                                 "error "
                                                 "code: %d. error message: %s.",
                                                 wrapper.errorCode(),
                                                 wrapper.errorMessage()));
        return;
      }
    }

  } else {
    if (!wrapper.prepare("INSERT INTO records (time, record_date, map, run, "
                         "user_id, player_name) VALUES (?, ?, ?, ?, ?, "
                         "?);")) {
      Printer::LogPrintln(
          ETJump::stringFormat("SaveRecord::couldn't prepare insert "
                               "statement. "
                               "error code: %d. error message: %s.",
                               wrapper.errorCode(), wrapper.errorMessage()));
      return;
    }

    if (!wrapper.bindInteger(1, record.time)) {
      Printer::LogPrintln(
          ETJump::stringFormat("SaveRecord::couldn't bind time to insert "
                               "statement. error code: %d. error message: "
                               "%s.",
                               wrapper.errorCode(), wrapper.errorMessage()));
      return;
    }

    if (!wrapper.bindInteger(2, record.date)) {
      Printer::LogPrintln(
          ETJump::stringFormat("SaveRecord::couldn't bind date to insert "
                               "statement. error code: %d. error message: "
                               "%s.",
                               wrapper.errorCode(), wrapper.errorMessage()));
      return;
    }

    if (!wrapper.bindText(3, record.map)) {
      Printer::LogPrintln(
          ETJump::stringFormat("SaveRecord::couldn't bind map to insert "
                               "statement. error code: %d. error message: "
                               "%s.",
                               wrapper.errorCode(), wrapper.errorMessage()));
      return;
    }

    if (!wrapper.bindText(4, record.run)) {
      Printer::LogPrintln(
          ETJump::stringFormat("SaveRecord::couldn't bind run to insert "
                               "statement. error code: %d. error message: "
                               "%s.",
                               wrapper.errorCode(), wrapper.errorMessage()));
      return;
    }

    if (!wrapper.bindInteger(5, record.userId)) {
      Printer::LogPrintln(
          ETJump::stringFormat("SaveRecord::couldn't user id to insert "
                               "statement. error code: %d. error message: "
                               "%s.",
                               wrapper.errorCode(), wrapper.errorMessage()));
      return;
    }

    if (!wrapper.bindText(6, record.playerName)) {
      Printer::LogPrintln(
          ETJump::stringFormat("SaveRecord::couldn't bind player name to "
                               "insert statement. error "
                               "code: %d. error message: %s.",
                               wrapper.errorCode(), wrapper.errorMessage()));
      return;
    }
  }

  if (!wrapper.execute()) {
    Printer::LogPrintln(
        ETJump::stringFormat("SaveRecord::couldn't execute insert or update "
                             "statement. error code: %d. error message: %s.",
                             wrapper.errorCode(), wrapper.errorMessage()));
    return;
  }
}

void Timerun::SaveRecord(Record *record, bool update) {
  std::thread thr(::SaveRecord, update, _database, *record);
  thr.detach();
}

void Timerun::addNewRecord(Player *player, int clientNum) {
  auto record = new Record();
  time_t currentTime;
  time(&currentTime);
  record->playerName = player->name;
  record->time = player->completionTime;
  record->date = static_cast<int>(currentTime);
  record->userId = player->userId;
  record->map = _currentMap;
  record->run = player->currentRunName;
  _recordsByName[player->currentRunName].push_back(
      std::unique_ptr<Record>(record));
  _sorted[player->currentRunName] = false;
  SaveRecord(record, false);
  Printer::SendCommandToAll(
      ETJump::stringFormat("record %d \"%s\" %d", clientNum,
                           player->currentRunName, player->completionTime));
}

void Timerun::updatePreviousRecord(Record *previousRecord, Player *player,
                                   int clientNum) {
  time_t currentTime;
  time(&currentTime);

  if (previousRecord->time > player->completionTime) {
    Printer::SendCommandToAll(
        ETJump::stringFormat("record %d \"%s\" %d", clientNum,
                             player->currentRunName, player->completionTime));

    previousRecord->time = player->completionTime;
    previousRecord->date = static_cast<int>(currentTime);
    previousRecord->playerName = player->name;
    _sorted[player->currentRunName] = false;
    SaveRecord(previousRecord, true);
  } else // Previous record was faster
  {
    Printer::SendCommandToAll(
        ETJump::stringFormat("completion %d \"%s\" %d", clientNum,
                             player->currentRunName, player->completionTime));
  }
}

Timerun::Record *Timerun::findPreviousRecord(Player *player) {
  auto run = _recordsByName.find(player->currentRunName);
  if (run == _recordsByName.end()) {
    return nullptr;
  }

  auto begin = std::begin(run->second);
  auto end = std::end(run->second);
  auto record =
      std::find_if(begin, end, [&player](const std::unique_ptr<Record> &r) {
        return r->run == player->currentRunName && r->userId == player->userId;
      });
  if (record != end) {
    return record->get();
  }
  return nullptr;
}

void Timerun::sortRecords() {
  for (auto &record : _recordsByName) {
    std::string runName = record.first;
    if (!_sorted[runName]) {
      std::sort(record.second.begin(), record.second.end(),
                [](const std::unique_ptr<Record> &lhs,
                   const std::unique_ptr<Record> &rhs) -> bool {
                  return lhs->time < rhs->time;
                });
      _sorted[runName] = true;
    }
  }
}

void Timerun::checkRecord(Player *player, int clientNum) {
  int currentBestTime = 0;

  // sort and grab the #1 records for each run before storing completion time
  // otherwise we might have our current run time as currentBestTime
  sortRecords();
  for (const auto &records : _recordsByName) {
    if (ETJump::sanitize(player->currentRunName, true) ==
        ETJump::sanitize(records.first, true)) {
      currentBestTime = records.second[0]->time;
    }
  }

  auto previousRecord = findPreviousRecord(player);
  if (previousRecord) {
    updatePreviousRecord(previousRecord, player, clientNum);
  } else {
    addNewRecord(player, clientNum);
  }

  // currentBestTime will be 0 for completely new records since records
  // are only registered on the first completion of any run, so the loop
  // above will not register anything for a run with no records
  if (player->completionTime < currentBestTime) {
    std::string cleanRunName = ETJump::sanitize(player->currentRunName, false);
    Printer::BroadCastBannerMessage(ETJump::stringFormat(
        "^7%s ^7broke the server record on ^3%s\n^7with ^3%s ^z(%s^z) ^7!!!\n",
        player->name, cleanRunName, millisToString(player->completionTime),
        diffToString(player->completionTime, currentBestTime)));
  }
}

bool Timerun::clientConnect(int clientNum, int userId) {
  _players[clientNum] = std::unique_ptr<Player>(new Player(userId));

  return true;
}

void Timerun::printRecords(int clientNum, const std::string &map,
                           const std::string &runName) {
  sortRecords();
  if (!map.length() || map == _currentMap) {
    if (!runName.length()) {
      printCurrentMapRecords(clientNum);
      return;
    }

    printRecordsForRun(clientNum, runName);
    return;
  }
  Printer::SendConsoleMessage(clientNum,
                              "This feature is not yet implemented.");
}
