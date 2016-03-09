//
// Created by Jussi on 5.4.2015.
//

#include <sqlite3.h>
#include <boost/format.hpp>
#include <ctime>
#include <thread>
#include <vector>
#include <memory>
#include <array>
#include <map>
#include "timerun.hpp"
#include "sqlite_wrapper.hpp"
#include "printer.hpp"
#include "Utilities.h"

std::string millisToString(int millis)
{
	std::string s;

	int minutes, seconds;

	minutes = millis / 60000;
	millis -= minutes * 60000;
	seconds = millis / 1000;
	millis -= seconds * 1000;

	s = (boost::format("%02d:%02d:%03d") % minutes % seconds % millis).str();

	return s;
}

std::string dateToFormat(int date)
{
	char buffer[128] = "\0";
	auto t           = static_cast<time_t>(date);
	strftime(buffer, sizeof(buffer), "%d.%m.%Y", localtime(&t));
	return buffer;
}

std::string GetColumnText(sqlite3_stmt *stmt, int index)
{
	auto text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, index));
	return text ? text : "";
}

bool Timerun::init(const std::string &database, const std::string &currentMap)
{
	SQLiteWrapper wrapper;

	Printer::LogPrintln("Opening timeruns database: " + database);

	_recordsByName.clear();
	for (auto &p : _players)
	{
		p = nullptr;
	}
	_currentMap = currentMap;
	_database   = database;
	for (auto &s : _sorted)
	{
		s.second = false;
	}

	if (!wrapper.open(database.c_str()))
	{
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
	                     "player_name TEXT NOT NULL);"))
	{
		_message = (boost::format("Timerun::init: couldn't prepare create table statement. error code: %d. error message: %s.")
		            % wrapper.errorCode() % wrapper.errorMessage()).str();
		return false;
	}

	if (!wrapper.execute())
	{
		_message = (boost::format("Timerun::init: couldn't execute statement. error code: %d. error message: %s.")
		            % wrapper.errorCode() % wrapper.errorMessage()).str();
		return false;
	}

	if (!wrapper.prepare("SELECT id, time, run, user_id, player_name, record_date FROM records WHERE map=?;"))
	{
		_message = (boost::format("Timerun::init: couldn't prepare select runs statement. error code: %d. error message: %s.")
		            % wrapper.errorCode() % wrapper.errorMessage()).str();
		return false;
	}

	if (!wrapper.bindText(1, currentMap))
	{
		_message = (boost::format("Timerun::init: couldn't bind current map to statement. error code: %d. error message: %s.")
		            % wrapper.errorCode() % wrapper.errorMessage()).str();
		return false;
	}

	auto stmt = wrapper.getStatement();
	auto rc   = 0;
	for (rc = sqlite3_step(stmt); rc == SQLITE_ROW; rc = sqlite3_step(stmt))
	{
		Record *record = new Record();

		record->id         = sqlite3_column_int(stmt, 0);
		record->time       = sqlite3_column_int(stmt, 1);
		record->run        = GetColumnText(stmt, 2);
		record->userId     = sqlite3_column_int(stmt, 3);
		record->playerName = GetColumnText(stmt, 4);
		record->date       = sqlite3_column_int(stmt, 5);
		record->map        = currentMap;

		_recordsByName[record->run].push_back(std::unique_ptr<Record>(record));
	}
	if (rc != SQLITE_DONE)
	{
		_message = (boost::format("Timerun::init: couldn't bind current map to statement. error code: %d. error message: %s")
		            % rc % wrapper.getSQLiteErrorMessage()).str();
		return false;
	}

	Printer::LogPrint((boost::format("Successfully loaded %d records from database\n") % _recordsByName.size()).str());

	return true;
}

void Timerun::startTimer(const std::string &runName, int clientNum, const std::string& currentName, int raceStartTime)
{
	auto player = _players[clientNum].get();

	if (player == nullptr)
	{
		return;
	}

	if (player->racing)
	{
		return;
	}

	player->racing         = true;
	player->name           = currentName;
	player->runStartTime   = raceStartTime;
	player->completionTime = 0;
	player->currentRunName = runName;
	startNotify(clientNum);

	Utilities::startRun(clientNum);
}

void Timerun::connectNotify(int clientNum)
{
	for (int idx = 0; idx < 64; ++idx)
	{
		auto player = _players[idx].get();
		if (player && player->currentRunName.length() > 0)
		{
			auto previousRecord = findPreviousRecord(player);

			int fastestCompletionTime = -1;
			if (previousRecord)
			{
				fastestCompletionTime = previousRecord->time;
			}
			Printer::SendCommand(clientNum, (boost::format("timerun start %d %d %s %d") % idx % player->runStartTime % player->currentRunName % fastestCompletionTime).str());
		}
	}
}

void Timerun::startNotify(int clientNum)
{
	auto player         = _players[clientNum].get();
	auto spectators     = Utilities::getSpectators(clientNum);
	auto previousRecord = findPreviousRecord(player);

	int fastestCompletionTime = -1;
	if (previousRecord)
	{
		fastestCompletionTime = previousRecord->time;
	}
	Printer::SendCommand(clientNum, (boost::format("timerun_start %d %s %d")
	                                 % (player->runStartTime)
	                                 % player->currentRunName
	                                 % fastestCompletionTime
	                                 ).str());
	Printer::SendCommand(spectators, (boost::format("timerun_start_spec %d %d %s %d")
	                                  % clientNum
	                                  % (player->runStartTime)
	                                  % (player->currentRunName)
	                                  % fastestCompletionTime
	                                  ).str());

	Printer::SendCommandToAll((boost::format("timerun start %d %d %s %d") % clientNum % player->runStartTime % player->currentRunName % fastestCompletionTime).str());
}

void Timerun::stopTimer(int clientNum, int commandTime, std::string runName)
{
	Player *player = _players[clientNum].get();

	if (player == nullptr)
	{
		return;
	}

	if (player->racing && player->currentRunName == runName)
	{
		auto millis = commandTime - player->runStartTime;

		player->completionTime = millis;
		checkRecord(player, clientNum);

		player->racing = false;

		Printer::SendCommand(clientNum, (boost::format("timerun_stop %d %s")
		                                 % millis
		                                 % player->currentRunName).str());
		auto spectators = Utilities::getSpectators(clientNum);
		Printer::SendCommand(spectators, (boost::format("timerun_stop_spec %d %d %s")
		                                  % clientNum
		                                  % millis
		                                  % player->currentRunName).str());

		Printer::SendCommandToAll((boost::format("timerun stop %d %d %s") % clientNum % millis % player->currentRunName).str());

		player->currentRunName = "";
		Utilities::stopRun(clientNum);
	}
}

std::string rankToString(int rank)
{
	switch (rank)
	{
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

void Timerun::printRecordsForRun(int clientNum, const std::string& runName)
{
	const auto run = _recordsByName.find(runName);
	if (run == end(_recordsByName))
	{
		Printer::SendConsoleMessage(clientNum,
		                            "^3error: ^7no records found by name: " + runName);
		return;
	}

	const auto self      = _players[clientNum].get();
	auto       foundSelf = false;

	std::string buffer =
	    "^g=============================================================\n"
	    " ^2Top 50 records for map: ^7" + _currentMap + "\n"
	                                                    "^g=============================================================\n";

	auto rank = 1;
	buffer += " ^2Run: ^7" + run->first + "\n\n";
	buffer += "^g Rank   Time        Player\n";

	for (auto&record:run->second)
	{
		if (rank <= 50)
		{
			if (record->userId == self->userId)
			{
				buffer   += (boost::format("^7 %5s   ^7 %s   %s ^7(^1You^7)\n") % rankToString(rank) % millisToString(record->time) % record->playerName).str();
				foundSelf = true;
			}
			else
			{
				buffer += (boost::format("^7 %5s   ^7 %s   %s\n") % rankToString(rank) % millisToString(record->time) % record->playerName).str();
			}
		}
		else
		{
			if (foundSelf)
			{
				break;
			}

			if (record->userId == self->userId)
			{
				buffer   += (boost::format("^7 %4s    ^7 %s   %s ^7(^1You^7)\n") % rankToString(rank) % millisToString(record->time) % record->playerName).str();
				foundSelf = true;
			}
		}
		rank++;
	}

	if (!foundSelf)
	{
		buffer += "^7You haven't set a record on this run yet!\n";
	}

	buffer += "^g=============================================================\n";
	Printer::SendConsoleMessage(clientNum, buffer);
}

void Timerun::printCurrentMapRecords(int clientNum)
{
	std::string buffer =
	    "^g=============================================================\n"
	    " ^2Top records for map: ^7" + _currentMap + "\n"
	                                                 "^g=============================================================\n";

	for (const auto&run : _recordsByName)
	{
		auto rank = 1;
		buffer += " ^2Run: ^7" + run.first + "\n\n";
		buffer += "^g Rank   Time        Player\n";
		for (const auto&record : run.second)
		{
			if (rank > 3)
			{
				break;
			}
			buffer += (boost::format("^7 %4s    ^7 %s   %s\n") % rankToString(rank++) % millisToString(record->time) % record->playerName).str();
		}

		buffer += "^g=============================================================\n";
	}

	Printer::SendConsoleMessage(clientNum, buffer);
}

void Timerun::interrupt(int clientNum)
{
	Player *player = _players[clientNum].get();

	if (player == nullptr)
	{
		return;
	}

	player->racing         = false;
	player->currentRunName = "";

	Utilities::stopRun(clientNum);
	Printer::SendCommand(clientNum, "timerun_interrupt");
	Printer::SendCommandToAll((boost::format("timerun interrupt %d") % clientNum).str());
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
	if (!wrapper.open(database))
	{
		Printer::LogPrintln((boost::format("SaveRecord: Couldn't open database to save the record. error code: %d. error message: %s.")
		                     % wrapper.errorCode() % wrapper.errorMessage()).str());
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

	if (update)
	{
		if (record.id != -1)
		{
			if (!wrapper.prepare("UPDATE records SET time=?, record_date=?, player_name=? WHERE id=?;"))
			{
				Printer::LogPrintln(
					(boost::format("SaveRecord::couldn't prepare update statement. error code: %d. error message: %s.")
						% wrapper.errorCode() % wrapper.errorMessage()).str());
				return;
			}

			if (!wrapper.bindInteger(1, record.time))
			{
				Printer::LogPrintln(
					(boost::format("SaveRecord::couldn't bind time to update statement. error code: %d. error message: %s.")
						% wrapper.errorCode() % wrapper.errorMessage()).str());
				return;
			}

			if (!wrapper.bindInteger(2, record.date))
			{
				Printer::LogPrintln(
					(boost::format("SaveRecord::couldn't bind date to update statement. error code: %d. error message: %s.")
						% wrapper.errorCode() % wrapper.errorMessage()).str());
				return;
			}

			if (!wrapper.bindText(3, record.playerName))
			{
				Printer::LogPrintln(
					(boost::format("SaveRecord::couldn't bind name to update statement. error code: %d. error message: %s.")
						% wrapper.errorCode() % wrapper.errorMessage()).str());
				return;
			}

			if (!wrapper.bindInteger(4, record.id))
			{
				Printer::LogPrintln(
					(boost::format("SaveRecord::couldn't bind id to update statement. error code: %d. error message: %s.")
						% wrapper.errorCode() % wrapper.errorMessage()).str());
				return;
			}
		} else
		{
			if (!wrapper.prepare("UPDATE records SET time=?, record_date=?, player_name=? WHERE map=? AND run=? and user_id=?;"))
			{
				Printer::LogPrintln(
					(boost::format("SaveRecord::couldn't prepare update statement. error code: %d. error message: %s.")
						% wrapper.errorCode() % wrapper.errorMessage()).str());
				return;
			}

			if (!wrapper.bindInteger(1, record.time))
			{
				Printer::LogPrintln(
					(boost::format("SaveRecord::couldn't bind time to update statement. error code: %d. error message: %s.")
						% wrapper.errorCode() % wrapper.errorMessage()).str());
				return;
			}

			if (!wrapper.bindInteger(2, record.date))
			{
				Printer::LogPrintln(
					(boost::format("SaveRecord::couldn't bind date to update statement. error code: %d. error message: %s.")
						% wrapper.errorCode() % wrapper.errorMessage()).str());
				return;
			}

			if (!wrapper.bindText(3, record.playerName))
			{
				Printer::LogPrintln(
					(boost::format("SaveRecord::couldn't bind name to update statement. error code: %d. error message: %s.")
						% wrapper.errorCode() % wrapper.errorMessage()).str());
				return;
			}

			if (!wrapper.bindText(4, record.map))
			{
				Printer::LogPrintln(
					(boost::format("SaveRecord::couldn't bind map name to update statement. error code: %d. error message: %s.")
						% wrapper.errorCode() % wrapper.errorMessage()).str());
				return;
			}

			if (!wrapper.bindText(5, record.run))
			{
				Printer::LogPrintln(
					(boost::format("SaveRecord::couldn't bind run name to update statement. error code: %d. error message: %s.")
						% wrapper.errorCode() % wrapper.errorMessage()).str());
				return;
			}

			if (!wrapper.bindInteger(6, record.userId))
			{
				Printer::LogPrintln(
					(boost::format("SaveRecord::couldn't bind user id to update statement. error code: %d. error message: %s.")
						% wrapper.errorCode() % wrapper.errorMessage()).str());
				return;
			}
		}
		
	}
	else
	{
		if (!wrapper.prepare("INSERT INTO records (time, record_date, map, run, user_id, player_name) VALUES (?, ?, ?, ?, ?, ?);"))
		{
			Printer::LogPrintln(
			    (boost::format("SaveRecord::couldn't prepare insert statement. error code: %d. error message: %s.")
			     % wrapper.errorCode() % wrapper.errorMessage()).str());
			return;
		}

		if (!wrapper.bindInteger(1, record.time))
		{
			Printer::LogPrintln(
			    (boost::format("SaveRecord::couldn't bind time to insert statement. error code: %d. error message: %s.")
			     % wrapper.errorCode() % wrapper.errorMessage()).str());
			return;
		}

		if (!wrapper.bindInteger(2, record.date))
		{
			Printer::LogPrintln(
			    (boost::format("SaveRecord::couldn't bind date to insert statement. error code: %d. error message: %s.")
			     % wrapper.errorCode() % wrapper.errorMessage()).str());
			return;
		}

		if (!wrapper.bindText(3, record.map))
		{
			Printer::LogPrintln(
			    (boost::format("SaveRecord::couldn't bind map to insert statement. error code: %d. error message: %s.")
			     % wrapper.errorCode() % wrapper.errorMessage()).str());
			return;
		}

		if (!wrapper.bindText(4, record.run))
		{
			Printer::LogPrintln(
			    (boost::format("SaveRecord::couldn't bind run to insert statement. error code: %d. error message: %s.")
			     % wrapper.errorCode() % wrapper.errorMessage()).str());
			return;
		}

		if (!wrapper.bindInteger(5, record.userId))
		{
			Printer::LogPrintln(
			    (boost::format("SaveRecord::couldn't user id to insert statement. error code: %d. error message: %s.")
			     % wrapper.errorCode() % wrapper.errorMessage()).str());
			return;
		}

		if (!wrapper.bindText(6, record.playerName))
		{
			Printer::LogPrintln(
			    (boost::format("SaveRecord::couldn't bind player name to insert statement. error code: %d. error message: %s.")
			     % wrapper.errorCode() % wrapper.errorMessage()).str());
			return;
		}
	}

	if (!wrapper.execute())
	{
		Printer::LogPrintln(
		    (boost::format("SaveRecord::couldn't execute insert or update statement. error code: %d. error message: %s.")
		     % wrapper.errorCode() % wrapper.errorMessage()).str());
		return;
	}
}

void Timerun::SaveRecord(Record *record, bool update)
{
	std::thread thr(::SaveRecord, update, _database, *record);
	thr.detach();
}

void Timerun::addNewRecord(Player *player, int clientNum)
{
	auto   record = new Record();
	time_t currentTime;
	time(&currentTime);
	record->playerName = player->name;
	record->time       = player->completionTime;
	record->date       = static_cast<int>(currentTime);
	record->userId     = player->userId;
	record->map        = _currentMap;
	record->run        = player->currentRunName;
	_recordsByName[player->currentRunName].push_back(std::unique_ptr<Record>(record));
	_sorted[player->currentRunName] = false;
	SaveRecord(record, false);
	Printer::SendCommandToAll((boost::format("record %d %s %d")
	                           % clientNum
	                           % player->currentRunName
	                           % player->completionTime).str());
}

void Timerun::updatePreviousRecord(Record *previousRecord, Player *player, int clientNum)
{
	time_t currentTime;
	time(&currentTime);

	if (previousRecord->time > player->completionTime)
	{
		Printer::SendCommandToAll((boost::format("record %d %s %d")
		                           % clientNum
		                           % player->currentRunName
		                           % player->completionTime).str());

		previousRecord->time            = player->completionTime;
		previousRecord->date            = static_cast<int>(currentTime);
		previousRecord->playerName      = player->name;
		_sorted[player->currentRunName] = false;
		SaveRecord(previousRecord, true);
	}
	else // Previous record was faster
	{
		Printer::SendCommandToAll((boost::format("completion %d %s %d")
		                           % clientNum
		                           % player->currentRunName
		                           % player->completionTime).str());
	}
}

Timerun::Record *Timerun::findPreviousRecord(Player *player)
{
	auto run = _recordsByName.find(player->currentRunName);
	if (run == _recordsByName.end())
	{
		return nullptr;
	}

	auto begin  = std::begin(run->second);
	auto end    = std::end(run->second);
	auto record = std::find_if(begin, end, [&player](const std::unique_ptr<Record>& r)
	{
		return r->run == player->currentRunName && r->userId == player->userId;
	});
	if (record != end)
	{
		return record->get();
	}
	return nullptr;
}

void Timerun::sortRecords()
{
	for (auto& record : _recordsByName)
	{
		std::string runName = record.first;
		if (!_sorted[runName])
		{
			std::sort(record.second.begin(), record.second.end(),
			          [](const std::unique_ptr<Record>& lhs,
			             const std::unique_ptr<Record>& rhs) -> bool {
				return lhs->time < rhs->time;
			});
			_sorted[runName] = true;
		}
	}
}

void Timerun::checkRecord(Player *player, int clientNum)
{
	Record *recordToUpdate = nullptr;
	bool   update          = false;
	time_t t;
	time(&t);

	int millis  = player->completionTime;
	int seconds = millis / 1000;
	millis = millis - seconds * 1000;
	int minutes = seconds / 60;
	seconds = seconds - minutes * 60;

	auto previousRecord = findPreviousRecord(player);
	if (previousRecord)
	{
		updatePreviousRecord(previousRecord, player, clientNum);
	}
	else
	{
		addNewRecord(player, clientNum);
	}

}

bool Timerun::clientConnect(int clientNum, int userId)
{
	_players[clientNum] = std::unique_ptr<Player>(new Player(userId));

	return true;
}



void Timerun::printRecords(int clientNum, const std::string &map, const std::string &runName)
{
	sortRecords();
	if (!map.length() || map == _currentMap)
	{
		if (!runName.length())
		{
			printCurrentMapRecords(clientNum);
			return;
		}

		printRecordsForRun(clientNum, runName);
		return;
	}
	Printer::SendConsoleMessage(clientNum, "This feature is not yet implemented.");
}
