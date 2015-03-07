#include "mapdata.h"
#include "g_utilities.hpp"
#include <boost/algorithm/string.hpp>

void MapData::Shutdown()
{
    time_t curr;
    time(&curr);

    std::string name = level.rawmapname;
    boost::to_lower(name);
    boost::replace_all(name, "'", "''");

    ConstNameIterator it = GetMapByName(name);
    if (it != GetNameIterEnd())
    {
        unsigned secondsPlayed = it->get()->secondsPlayed;
        sqlite3_stmt *stmt = NULL;
        if (!PrepareStatement("UPDATE maps SET last_played=?, seconds_played=? WHERE map=?;", &stmt))
        {
            sqlite3_close(db_);
            return;
        }

        secondsPlayed += static_cast<unsigned>(curr)-static_cast<unsigned>(mapStartTime_);

        if (!BindInt(stmt, 1, static_cast<unsigned>(curr)) ||
            !BindInt(stmt, 2, secondsPlayed) ||
            !BindString(stmt, 3, name.c_str()))
        {
            sqlite3_close(db_);
            G_LogPrintf("Failed to update map statistics: %s.\n", sqlite3_errmsg(db_));
            return;
        }

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            G_LogPrintf("ERROR while updating map last played: %s\n", sqlite3_errmsg(db_));
        }
        sqlite3_finalize(stmt);
    }
    else {
        G_LogPrintf("Couldn't find map with name %s\n", level.rawmapname);
    }

    maps_.clear();

    sqlite3_close(db_);
}

bool MapData::Initialize()
{
    // Just to make sure
    maps_.clear();
    int rc = sqlite3_open(GetPath("maps.db").c_str(), &db_);
    if (rc != SQLITE_OK)
    {
        message_ = sqlite3_errmsg(db_);
        return false;
    }
    sqlite3_exec(db_, "PRAGMA journal_mode=WAL;",
        NULL, NULL, NULL);

    sqlite3_stmt *stmt;
    if (!PrepareStatement("CREATE TABLE IF NOT EXISTS maps (id INTEGER PRIMARY KEY AUTOINCREMENT, map TEXT UNIQUE, last_played INTEGER, seconds_played INTEGER);", &stmt))
    {
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
        return false;
    }

    sqlite3_finalize(stmt);

    if (!PrepareStatement("SELECT id, map, last_played, seconds_played FROM maps;", &stmt))
    {
        return false;
    }

    mapNames_.clear();
    const char *val = NULL;
    rc = sqlite3_step(stmt);
    while (rc == SQLITE_ROW)
    {
        Map newMap(new Map_s);

        newMap->id = sqlite3_column_int(stmt, 0);
        val = (const char*)(sqlite3_column_text(stmt, 1));
        newMap->name = val ? val : "";
        newMap->lastPlayed = sqlite3_column_int(stmt, 2);
        newMap->secondsPlayed = sqlite3_column_int(stmt, 3);

        G_DPrintf("Loaded map: %s\n", newMap->ToChar());

        maps_.insert(newMap);

        rc = sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);

    int numDirs = 0;
    char dirList[8192] = "\0";
    char *dirPtr = NULL;
    int i = 0;
    int dirLen = 0;

    numDirs = trap_FS_GetFileList("maps", ".bsp",
        dirList, sizeof(dirList));
    dirPtr = dirList;

    sqlite3_exec(db_, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    for (i = 0; i < numDirs; i++, dirPtr += dirLen + 1)
    {
        dirLen = strlen(dirPtr);
        if (strlen(dirPtr) > 4)
        {
            dirPtr[strlen(dirPtr) - 4] = '\0';
        }

        char buf[MAX_QPATH] = "\0";
        Q_strncpyz(buf, dirPtr, sizeof(buf));
        boost::to_lower(buf);

        mapNames_.push_back(buf);

        NameIterator it = GetMapByName(buf);
        if (it == GetNameIterEnd())
        {
            Map newMap(new Map_s);
            
            // Hack that will not allow 64 char names with ' in them
            if (strlen(buf) != MAX_QPATH)
            {
                if (buf)
                {
                    std::string temp = buf;
                    boost::replace_all(temp, "'", "''");
                    Q_strncpyz(buf, temp.c_str(), sizeof(buf));
                }
            }

            newMap->name = buf;
            maps_.insert(newMap);
            it = GetMapByName(buf);
            int rc = sqlite3_exec(db_, va("INSERT INTO maps (map, last_played, seconds_played) VALUES ('%s', 0, 0);", buf),
                NULL, NULL, NULL);
            newMap->id = sqlite3_last_insert_rowid(db_);
            if (rc != SQLITE_OK)
            {
                G_LogPrintf("Error while trying to insert a map to database: %s\n", sqlite3_errmsg(db_));
            }
        }
        it->get()->mapOnServer = true;
    }
    sqlite3_exec(db_, "END TRANSACTION;", NULL, NULL, NULL);

    time(&mapStartTime_);

    return true;
}

void MapData::ListMaps(gentity_t *ent)
{
    ChatPrintTo(ent, "^3listmaps: ^7check console for more information.");
    BeginBufferPrint();

    int printed = 1;
    ConstNameIterator it = maps_.get<0>().begin();
    ConstNameIterator end = maps_.get<0>().end();
    while (it != end)
    {
        if (!it->get()->mapOnServer)
        {
            it++;
            continue;
        }

        BufferPrint(ent, va("%-25s ", it->get()->name.c_str()));

        if (printed != 0 && printed % 3 == 0)
        {
            printed = 0;
            BufferPrint(ent, "\n");
        }

        it++;
        printed++;
    }
    FinishBufferPrint(ent, true);
}

void MapData::ListMostPlayed(gentity_t *ent)
{
    ChatPrintTo(ent, "^3mostplayed: ^7check console for more information.");

    ConstRSecondsPlayedIterator it = maps_.get<2>().rbegin();
    ConstRSecondsPlayedIterator end = maps_.get<2>().rend();
    int printed = 1;
    BeginBufferPrint();
    while (it != end && printed < 6)
    {
        BufferPrint(ent, va("%-25s %s\n", it->get()->name.c_str(), TimeStampDifferenceToString(it->get()->secondsPlayed).c_str()));

        printed++;
        it++;
    }

    FinishBufferPrint(ent, true);

}

void MapData::PrintMapInfo(gentity_t* ent, const std::string& mapName)
{
    std::string map = mapName;
    boost::to_lower(map);
    bool currentMap = false;

    if (!Q_stricmp(map.c_str(), level.rawmapname))
    {
        currentMap = true;
    }

    ConstNameIterator it = maps_.get<0>().find(map);
    if (it != maps_.get<0>().end())
    {
        unsigned seconds = 0;
        unsigned lastPlayed = 0;
        if (currentMap)
        {
            time_t current;
            time(&current);
            seconds = it->get()->secondsPlayed +
                static_cast<unsigned>((current-mapStartTime_));
        }
        else
        {
            lastPlayed = it->get()->lastPlayed;
            seconds = it->get()->secondsPlayed;
        }

        
        const unsigned SECONDS_IN_DAY = 60 * 60 * 24;
        const unsigned SECONDS_IN_HOUR = 60 * 60;
        const unsigned SECONDS_IN_MINUTE = 60;
        unsigned days = seconds / SECONDS_IN_DAY;
        seconds = seconds - days * SECONDS_IN_DAY;
        unsigned hours = seconds / SECONDS_IN_HOUR;
        seconds = seconds - hours * SECONDS_IN_HOUR;
        unsigned minutes = seconds / SECONDS_IN_MINUTE;
        seconds = seconds - minutes * SECONDS_IN_MINUTE;

        if (currentMap)
        {
            ChatPrintTo(ent, va("^3mapinfo: ^7%s is the current map on the server. It has been played for a total of %d days %d hours %d minutes and %d seconds.",
                map.c_str(), days, hours, minutes, seconds));
        }
        else
        {
            ChatPrintTo(ent, va("^3mapinfo: ^7%s was last played in %s. It has been played for a total of %d days %d hours %d minutes and %d seconds.",
                map.c_str(), TimeStampToString(lastPlayed).c_str(), days, hours, minutes, seconds));
        }
        
        return;
    }

    ChatPrintTo(ent, "^3mapinfo: ^7couldn't find a map with name " + mapName);
}

void MapData::ListLeastPlayed(gentity_t *ent)
{
    ChatPrintTo(ent, "^3leastplayed: ^7check console for more information.");

    ConstSecondsPlayedIterator it = maps_.get<2>().begin();
    ConstSecondsPlayedIterator end = maps_.get<2>().end();
    int printed = 1;
    BeginBufferPrint();
    while (it != end && printed < 6)
    {
        BufferPrint(ent, va("%-25s %s\n", it->get()->name.c_str(), TimeStampDifferenceToString(it->get()->secondsPlayed).c_str()));

        printed++;
        it++;
    }

    FinishBufferPrint(ent, true);
}

bool MapData::BindInt(sqlite3_stmt* stmt, int index, int val)
{
    int rc = sqlite3_bind_int(stmt, index, val);
    if (rc != SQLITE_OK)
    {
        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
        return false;
    }
    return true;
}

bool MapData::BindString(sqlite3_stmt* stmt, int index, std::string const& val)
{
    int rc = sqlite3_bind_text(stmt, index, val.c_str(), val.length(), SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
        return false;
    }
    return true;
}

MapData::ConstNameIterator MapData::GetNameIterEnd()
{
    return maps_.get<0>().end();
}

MapData::ConstLastPlayedIterator MapData::GetLPIterEnd()
{
    return maps_.get<1>().end();
}

MapData::NameIterator MapData::GetMapByName(std::string const& name)
{
    return maps_.get<0>().find(name);
}

MapData::LastPlayedIterator MapData::GetMapByLP(unsigned lastPlayed)
{
    return maps_.get<1>().find(lastPlayed);
}

MapData::SecondsPlayedIterator MapData::GetMapBySP(unsigned secondsPlayed)
{
    return maps_.get<2>().find(secondsPlayed);
}

MapData::ConstSecondsPlayedIterator MapData::GetSPIterEnd()
{
    return maps_.get<2>().end();
}

bool MapData::PrepareStatement(char const* query, sqlite3_stmt** stmt)
{
    unsigned rc = sqlite3_prepare_v2(db_, query, -1, stmt, 0);

    if (rc != SQLITE_OK)
    {
        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
        return false;
    }

    return true;
}

std::string MapData::GetMessage()
{
    return message_;
}

std::string MapData::RandomMap()
{
    int mapIndex = rand() % mapNames_.size();

    while (mapNames_[mapIndex] == level.rawmapname)
    {
      mapIndex = rand() % mapNames_.size();
    }

    return mapNames_[mapIndex];
}
