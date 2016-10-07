#include "etj_map_stats.h"
#include <boost/format.hpp>
#include <SQLiteCpp/Database.h>
#include <boost/algorithm/string.hpp>
#include "etj_map.h"
#include "etj_file.h"
#include <SQLiteCpp/Transaction.h>
#include "etj_log.h"

ETJump::MapStats::MapStats(const std::string& database, const std::string& currentMap, ISyscallsFacade *syscallsFacade):
	_database(File::getPath(database)),
	_currentMapName(currentMap),
	_currentMapsOnServer(getCurrentMaps()),
	_syscallsFacade(syscallsFacade)
{
	if (_syscallsFacade == nullptr)
	{
		Log::error("syscallsFacade is null.");
		return;
	}

	createDatabase();
	insertNewMaps();
}

ETJump::MapStats::~MapStats()
{
}

bool ETJump::MapStats::mapExists(const std::string& map) const
{
	return find(begin(_currentMapsOnServer), end(_currentMapsOnServer), map) != end(_currentMapsOnServer);
}

std::vector<std::string> ETJump::MapStats::maps() const
{
	return _currentMapsOnServer;
}

std::string ETJump::MapStats::currentMap() const
{
	return _currentMapName;
}

std::vector<std::string> ETJump::MapStats::matches(const std::string& map) const
{
	std::vector<std::string> matches;
	for (const auto & m : _currentMapsOnServer)
	{
		if (m.find(map) != std::string::npos)
		{
			matches.push_back(m);
		}
	}
	return matches;
}

void ETJump::MapStats::changeMap(const std::string& newMap)
{
	_syscallsFacade->execConsoleCommand(ISyscallsFacade::CbufExec::Append, "map " + newMap);
}

void ETJump::MapStats::createDatabase()
{
	try
	{
		SQLite::Database db(_database, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		db.exec(R"(
			CREATE TABLE IF NOT EXISTS map_statistics2 (
			  id             INTEGER PRIMARY KEY AUTOINCREMENT,
			  name           TEXT UNIQUE,
			  seconds_played INTEGER NOT NULL,
			  callvoted      INTEGER NOT NULL,
			  votes_passed   INTEGER NOT NULL,
			  times_played   INTEGER NOT NULL,
			  last_played    INTEGER NOT NULL
			);
		)");
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error((boost::format("could not create database for map statistics: %s") % e.what()).str());
	}
}

std::vector<std::string> ETJump::MapStats::getCurrentMaps()
{
	return File::fileList("maps", ".bsp");
}

void ETJump::MapStats::insertNewMaps()
{
	try
	{
		SQLite::Database db(_database, SQLite::OPEN_READWRITE);

		// get currently stored maps from database
		SQLite::Statement query(db, R"(
			SELECT
			  name
			FROM map_statistics2;
		)");

		std::vector<std::string> dbMaps{};
		while (query.executeStep())
		{
			dbMaps.push_back(query.getColumn(0));
		}

		// find new maps 
		std::vector<std::string> newMaps{};
		for (auto& map : _currentMapsOnServer)
		{
			if (find(begin(dbMaps), end(dbMaps), map) == end(dbMaps))
			{
				newMaps.push_back(map);
			}
		}

		// insert new maps
		SQLite::Transaction tx(db);
		SQLite::Statement insertStmt(db, R"(
			INSERT INTO map_statistics2 (name, seconds_played, callvoted, votes_passed, times_played, last_played)
			VALUES (:name, 0, 0, 0, 0, 0);
		)");
		for (auto& map : newMaps)
		{
			insertStmt.bind(":name", map);
			insertStmt.exec();
			insertStmt.clearBindings();
			insertStmt.reset();
		}
		tx.commit();
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error((boost::format("could not insert new maps to database: %s") % e.what()).str());
	}
}
