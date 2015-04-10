//
// Created by Jussi on 5.4.2015.
//

#include <sqlite3.h>
#include <boost/format.hpp>
#include <ctime>
#include <thread>
#include <string>
#include <vector>
#include <memory>
#include <array>
#include <map>
#include "Timerun.h"
#include "SQLiteWrapper.h"
#include "Printer.h"
#include "Utilities.h"

std::string GetColumnText(sqlite3_stmt *stmt, int index)
{
    const char *text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, index));
    return text ? text : "";
}

bool Timerun::init(const std::string &database, const std::string &currentMap)
{
    SQLiteWrapper wrapper;

    _records.clear();
    for (auto &p : _players) {
        p = nullptr;
    }
    _currentMap = currentMap;
    _database = database;
    for (auto &s : _sorted) {
        s.second = false;
    }

    if (!wrapper.open(database.c_str())) {
        _message = (boost::format("Timerun::init: couldn't open database. error code: %d. error message: %s.")
                    % wrapper.errorCode() % wrapper.errorMessage()).str();
        return false;
    }

    if (!wrapper.prepare("CREATE TABLE IF NOT EXISTS records (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                 "time INT NOT NULL, "
                                 "record_date INT NOT NULL, "
                                 "map TEXT NOT NULL, "
                                 "run TEXT NOT NULL, "
                                 "user_id INT NOT NULL, "
                                 "player_name TEXT NOT NULL);")) {
        _message = (boost::format("Timerun::init: couldn't prepare create table statement. error code: %d. error message: %s.")
                    % wrapper.errorCode() % wrapper.errorMessage()).str();
        return false;
    }

    if (!wrapper.execute()) {
        _message = (boost::format("Timerun::init: couldn't execute statement. error code: %d. error message: %s.")
                    % wrapper.errorCode() % wrapper.errorMessage()).str();
        return false;
    }

    if (!wrapper.prepare("SELECT id, time, run, user_id, player_name FROM records WHERE map=?;")) {
        _message = (boost::format("Timerun::init: couldn't prepare select runs statement. error code: %d. error message: %s.")
                    % wrapper.errorCode() % wrapper.errorMessage()).str();
        return false;
    }

    if (!wrapper.bindText(1, currentMap)) {
        _message = (boost::format("Timerun::init: couldn't bind current map to statement. error code: %d. error message: %s.")
                    % wrapper.errorCode() % wrapper.errorMessage()).str();
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
        record->map = currentMap;

        _records[record->run].push_back(std::unique_ptr<Record>(record));
    }
    if (rc != SQLITE_DONE) {
        _message = (boost::format("Timerun::init: couldn't bind current map to statement. error code: %d.")
                    % rc % wrapper.getSQLiteErrorMessage()).str();
        return false;
    }

    Printer::LogPrint((boost::format("Successfully loaded %d records from database\n") % _records.size()).str());

    return true;
}

void Timerun::startTimer(const std::string &runName, int clientNum, const std::string& currentName, int raceStartTime)
{
    Player *player = _players[clientNum].get();

    if (player == nullptr)
    {
        return;
    }

    if (!player->racing) {
        player->racing = true;
        player->name = currentName;
        player->raceStartTime = raceStartTime;
        player->completionTime = 0;
        player->currentRunName = runName;
        Printer::SendBannerMessage(clientNum, (boost::format("^7Run %s ^7started!") % player->currentRunName).str());
        auto spectators = Utilities::getSpectators(clientNum);
        Printer::SendCommand(clientNum, (boost::format("timerun_start %d")
                                         % (player->raceStartTime + 500)).str());
        Printer::SendCommand(spectators, (boost::format("timerun_start_spec %d %d")
                                          % clientNum
                                          % (player->raceStartTime + 500)).str());
        Utilities::startRun(clientNum);
    }
}

void Timerun::stopTimer(const std::string &runName, int clientNum, int commandTime)
{
    Player *player = _players[clientNum].get();

    if (player == nullptr)
    {
        return;
    }

    if (player->racing) {
        int millis = commandTime - player->raceStartTime;

        player->completionTime = millis;
        checkRecord(player, clientNum);

        player->racing = false;
        player->currentRunName = "";

        Printer::SendCommand(clientNum, (boost::format("timerun_stop %d")
                                         % millis).str());
        Printer::SendCommand(Utilities::getSpectators(clientNum), (boost::format("timerun_stop_spec %d %d")
                                                                   % clientNum
                                                                   % millis).str());
        Utilities::stopRun(clientNum);
    }
}

void Timerun::interrupt(int clientNum)
{
    Player *player = _players[clientNum].get();

    if (player == nullptr) {
        return;
    }

    player->racing = false;
    player->currentRunName = "";

    Utilities::stopRun(clientNum);
    Printer::SendCommand(clientNum, "timerun_stop");
}

/**
 * Saves the record to database
 * @param update Inserts if this is set to false, else update
 * @param database The database file name
 * @param record The actual record
 */
static void SaveRecord(bool update, std::string database, Timerun::Record record)
{
    SQLiteWrapper wrapper;
    if (!wrapper.open(database)) {
        // TODO: print error
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
        if (!wrapper.prepare("UPDATE records SET time=?, record_date=?, player_name=? WHERE id=?;")) {
            Printer::LogPrintln(
                    (boost::format("SaveRecord::couldn't prepare update statement. error code: %d. error message: %s.")
                     % wrapper.errorCode() % wrapper.errorMessage()).str());
            return;
        }

        if (!wrapper.bindInteger(1, record.time)) {
            Printer::LogPrintln(
                    (boost::format("SaveRecord::couldn't bind time to update statement. error code: %d. error message: %s.")
                     % wrapper.errorCode() % wrapper.errorMessage()).str());
            return;
        }

        if (!wrapper.bindInteger(2, record.date)) {
            Printer::LogPrintln(
                    (boost::format("SaveRecord::couldn't bind date to update statement. error code: %d. error message: %s.")
                     % wrapper.errorCode() % wrapper.errorMessage()).str());
            return;
        }

        if (!wrapper.bindText(3, record.playerName)) {
            Printer::LogPrintln(
                    (boost::format("SaveRecord::couldn't bind name to update statement. error code: %d. error message: %s.")
                     % wrapper.errorCode() % wrapper.errorMessage()).str());
            return;
        }

        if (!wrapper.bindInteger(4, record.id)) {
            Printer::LogPrintln(
                    (boost::format("SaveRecord::couldn't bind id to update statement. error code: %d. error message: %s.")
                     % wrapper.errorCode() % wrapper.errorMessage()).str());
            return;
        }
    } else {
        if (!wrapper.prepare("INSERT INTO records (time, record_date, map, run, user_id, player_name) VALUES (?, ?, ?, ?, ?, ?);"))  {
            Printer::LogPrintln(
                    (boost::format("SaveRecord::couldn't prepare insert statement. error code: %d. error message: %s.")
                     % wrapper.errorCode() % wrapper.errorMessage()).str());
            return;
        }

        if (!wrapper.bindInteger(1, record.time)) {
            Printer::LogPrintln(
                    (boost::format("SaveRecord::couldn't bind time to insert statement. error code: %d. error message: %s.")
                     % wrapper.errorCode() % wrapper.errorMessage()).str());
            return;
        }

        if (!wrapper.bindInteger(2, record.date)) {
            Printer::LogPrintln(
                    (boost::format("SaveRecord::couldn't bind date to insert statement. error code: %d. error message: %s.")
                     % wrapper.errorCode() % wrapper.errorMessage()).str());
            return;
        }

        if (!wrapper.bindText(3, record.map)) {
            Printer::LogPrintln(
                    (boost::format("SaveRecord::couldn't bind map to insert statement. error code: %d. error message: %s.")
                     % wrapper.errorCode() % wrapper.errorMessage()).str());
            return;
        }

        if (!wrapper.bindText(4, record.run)) {
            Printer::LogPrintln(
                    (boost::format("SaveRecord::couldn't bind run to insert statement. error code: %d. error message: %s.")
                     % wrapper.errorCode() % wrapper.errorMessage()).str());
            return;
        }

        if (!wrapper.bindInteger(5, record.userId)) {
            Printer::LogPrintln(
                    (boost::format("SaveRecord::couldn't user id to insert statement. error code: %d. error message: %s.")
                     % wrapper.errorCode() % wrapper.errorMessage()).str());
            return;
        }

        if (!wrapper.bindText(6, record.playerName)) {
            Printer::LogPrintln(
                    (boost::format("SaveRecord::couldn't bind player name to insert statement. error code: %d. error message: %s.")
                     % wrapper.errorCode() % wrapper.errorMessage()).str());
            return;
        }
    }

    if (!wrapper.execute()) {
        Printer::LogPrintln(
                (boost::format("SaveRecord::couldn't execute insert or update statement. error code: %d. error message: %s.")
                 % wrapper.errorCode() % wrapper.errorMessage()).str());
        return;
    }
}

bool Timerun::checkRecord(Player *player, int clientNum)
{
    Record *recordToUpdate = nullptr;
    bool update = false;
    time_t t;
    time(&t);

    int millis = player->completionTime;
    int seconds = millis / 1000;
    millis = millis - seconds * 1000;
    int minutes = seconds / 60;
    seconds = seconds - minutes * 60;

    for (auto& run : _records) {
        for (auto& record : run.second) {
            if (record->userId == player->userId) {
                // New time is faster
                if (player->completionTime < record->time) {
                    record->time = player->completionTime;
                    record->date = static_cast<int>(t);
                    record->playerName = player->name;
                    recordToUpdate = record.get();
                    update = true;



                    Printer::BroadcastBannerMessage((boost::format("%s ^7completed %s in %02d:%02d:%03d")
                                                     % player->name
                                                     % player->currentRunName
                                                     % minutes
                                                     % seconds
                                                     % millis).str());
                    // Old time was faster, inform client about the new time anyway
                } else {
                    Printer::SendBannerMessage(clientNum, (boost::format("You completed %s ^7in %02d:%02d:%03d")
                                                           % player->currentRunName
                                                           % minutes
                                                           % seconds
                                                           % millis).str());
                    return true;
                }
            }
        }
    }

    if (!recordToUpdate) {
        recordToUpdate = new Record();
        recordToUpdate->playerName = player->name;
        recordToUpdate->time = player->completionTime;
        recordToUpdate->date = static_cast<int>(t);
        recordToUpdate->userId = player->userId;
        recordToUpdate->map = _currentMap;
        recordToUpdate->run = player->currentRunName;
        _records[player->currentRunName].push_back(std::unique_ptr<Record>(recordToUpdate));
    }

    _sorted[player->currentRunName] = false;
    std::thread thr(::SaveRecord, update, _database, *recordToUpdate);
    thr.detach();

    return true;
}

bool Timerun::clientConnect(int clientNum, int userId)
{
    _players[clientNum] = std::unique_ptr<Player>(new Player(userId));

    return true;
}

void Timerun::printRecords(int clientNum, const std::string &map, const std::string &runName)
{
    if (runName.length() == 0) {
        Printer::SendConsoleMessage(clientNum, "^3error: ^7run name must be specified.");
        return;
    }

    // User wants to see the records of the current map
    if (map.length() == 0 || map == _currentMap) {
        auto run = _records.find(runName);
        if (run == _records.end()) {
            Printer::SendConsoleMessage(clientNum,
                                        (boost::format("^3error: ^7no records found by name %s.") % runName).str());
            return;
        }

        if (!_sorted[runName]) {
            std::sort(run->second.begin(), run->second.end());
            _sorted[runName] = true;
        }

        std::string runRecords;
        runRecords += (boost::format("^zTop 50 records for %s\n")
                       % runName).str();
        auto rank = 1;
        for (auto &record : run->second) {
            runRecords += (boost::format("^7%d ^7%s ^7%d %d\n")
                           % rank
                           % record->playerName
                           % record->time
                           % record->date).str();
            ++rank;
            if (rank == 50) {
                break;
            }
        }

        Printer::SendConsoleMessage(clientNum, runRecords);

        // User wants to see the records of some other map
    } else {
        Printer::SendConsoleMessage(clientNum, "This feature is not yet implemented.");
    }
}
