#include "mapdata.h"
#include "../g_local.hpp"
#include "../g_utilities.hpp"
#include <boost/algorithm/string.hpp>

MapData::MapData(): mapLoadTime_(0)
{

}

void MapData::Initialize()
{
    if(OpenMapDatabase())
    {
        if(CreateMapsTable())
        {
            LoadMapData(); 
        }
    }

    int numDirs = 0;
    char dirList[8192] = "\0";
    char *dirPtr = NULL;
    int i = 0;
    int dirLen = 0;

    numDirs = trap_FS_GetFileList("maps", ".bsp", 
        dirList, sizeof(dirList));
    dirPtr = dirList;

    sqlite3_exec(db_, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    for(i = 0; i < numDirs; i++, dirPtr += dirLen + 1)
    {
        dirLen = strlen(dirPtr);
        if(strlen(dirPtr) > 4)
        {
            dirPtr[strlen(dirPtr) - 4] = '\0';
        }

        char buf[MAX_QPATH] = "\0";
        Q_strncpyz(buf, dirPtr, sizeof(buf));
        boost::to_lower(buf);

        std::map<std::string, MapPtr>::iterator it = 
            maps_.find(buf);

        if(it == maps_.end())
        {
            MapPtr temp = MapPtr(new Map);
            it = maps_.insert(std::make_pair(buf, temp)).first;
            int rc = sqlite3_exec(db_, va("INSERT INTO maps (map, last_played, seconds_played) VALUES ('%s', 0, 0);",
                buf), NULL, NULL, NULL);
            if(rc != SQLITE_OK)
            {
                G_LogPrintf("ERROR while trying to insert a map to database: (%d) %s\n",
                    rc, sqlite3_errmsg(db_));
            }
        } 

        it->second->isOnServer = true;
    }
    sqlite3_exec(db_, "END TRANSACTION;", NULL, NULL, NULL);

    time_t t;
    if(time(&t))
    {
        mapLoadTime_ = static_cast<int>(t);
    } else
    {
        G_LogPrintf("ERROR: couldn't get current time in MapData::Initialize.\n");
    }
}

void MapData::Shutdown()
{
    UpdateLastPlayed();
    // shared_ptr handles memory management
    maps_.clear();
    sqlite3_close(db_);
    db_ = NULL;
}

std::string MapData::RandomMap()
{
    int index = rand() % maps_.size();
    std::map<std::string, MapPtr>::iterator it = maps_.begin();

    // Can't index it :|
    for(int i = 0; i != index; i++)
    {
        if(it == maps_.end())
        {
            G_LogPrintf("ERROR: out of bound indexing on map database.\n");
            return "";
        }
        it++;
    }

    if(it == maps_.end())
    {
        G_LogPrintf("ERROR: out of bound indexing on map database.\n");
        return "";
    }

    return it->first;
}

void MapData::ListMaps(gentity_t* ent)
{
    std::map<std::string, MapPtr>::const_iterator it =
        maps_.begin();
    
    BeginBufferPrint();
    const int MAPS_PER_LINE = 2;
    int mapsOnThisLine = 0;
    int numListedMaps = 0;
    while(it != maps_.end())
    {
        if(it->second->isOnServer)
        {
            if(mapsOnThisLine == MAPS_PER_LINE)
            {
                BufferPrint(ent, va("%-27s \n", it->first.c_str()));
                mapsOnThisLine = 0;
            } else
            {
                BufferPrint(ent, va("%-27s", it->first.c_str()));
                mapsOnThisLine++;
            }    
            numListedMaps++;
        }
        it++;
    }
    FinishBufferPrint(ent, true);

    ChatPrintTo(ent, va("^3listmaps: ^7listed %d maps.", numListedMaps));
}

void MapData::PrintMapInfo(gentity_t* ent, std::string const& name)
{
    std::map<std::string, MapPtr>::const_iterator it =
        maps_.find(name);
    if(it == maps_.end())
    {
        ChatPrintTo(ent, "^3mapinfo: ^7couldn't find map " + name + ".");
        return;
    }

    bool currentMap = false;
    if(!Q_stricmp(it->first.c_str(), level.rawmapname))
    {
        time_t t;
        if(!time(&t))
        {
            // Should never happen
            G_LogPrintf("ERROR: couldn't get time.\n");
        }
        it->second->played += static_cast<int>(t) - mapLoadTime_;
        mapLoadTime_ = static_cast<int>(t);
        currentMap = true;
    }

    int seconds = it->second->played;
    const int SECONDS_IN_DAY = 60 * 60 * 24;
    const int SECONDS_IN_HOUR = 60 * 60;
    const int SECONDS_IN_MINUTE = 60;
    int days = seconds / SECONDS_IN_DAY;
    seconds = seconds - days * SECONDS_IN_DAY;
    int hours = seconds / SECONDS_IN_HOUR;
    seconds = seconds - hours * SECONDS_IN_HOUR;
    int minutes = seconds / SECONDS_IN_MINUTE;
    seconds = seconds - minutes * SECONDS_IN_MINUTE;    

    if(!currentMap)
    {
        ChatPrintTo(ent, va("^3mapinfo: ^7%s was last played in %s. It has been played for a total of %d days %d hours %d minutes and %d seconds.",
        name.c_str(), TimeStampToString(it->second->lastPlayed).c_str(), days, hours, minutes, seconds));
    } else
    {
        ChatPrintTo(ent, va("^3mapinfo: ^7%s is the current map on the server. It has been played for a total of %d days %d hours %d minutes and %d seconds.",
        name.c_str(), days, hours, minutes, seconds));
    }
}

bool MapData::OpenMapDatabase()
{
    std::string file = "maps.dat";
    if(strlen(g_mapDatabase.string) > 0)
    {
        file = g_mapDatabase.string;
    }

    int rc = sqlite3_open(GetPath(file).c_str(), &db_);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't open map database %s: (%d) %s\n",
            file.c_str(), rc, sqlite3_errmsg(db_));
        return false;
    }

    return true;
}

bool MapData::CreateMapsTable()
{
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db_, 
        "CREATE TABLE IF NOT EXISTS maps("
        "id INTEGER PRIMARY KEY,"
        "map VARCHAR(255),"
        "last_played INTEGER,"
        "seconds_played INTEGER"
        ");",
        -1, &stmt, 0);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing createMapsTable statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE)
    {
        G_LogPrintf("Createing table \"maps\" failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

void MapData::LoadMapData()
{
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db_, 
        "SELECT map, last_played, seconds_played FROM maps;",
        -1, &stmt, 0);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing selectAllMaps statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return;
    }

    rc = sqlite3_step(stmt);
    while(rc == SQLITE_ROW)
    {
        int index = 0;
        const char *text = NULL;
        std::string name = "";
        MapPtr temp = MapPtr(new Map);

        text = (const char*)(sqlite3_column_text(stmt, index++));
        CharPtrToString(text, name);

        temp->lastPlayed = sqlite3_column_int(stmt, index++);
        temp->played = sqlite3_column_int(stmt, index++);

        maps_.insert(std::make_pair(name, temp));

        rc = sqlite3_step(stmt);
    }
    if( rc == SQLITE_DONE )
    {
        sqlite3_finalize(stmt);
        return;
    } else
    {
        G_LogPrintf("Couldn't read all maps from the database: (%d) (%s)\n",
            rc, sqlite3_errmsg(db_));
    }
    sqlite3_finalize(stmt);
}

void MapData::UpdateLastPlayed()
{
    time_t t;
    if(!time(&t))
    {
        return;
    }
    int currentTime = static_cast<int>(t);

    std::map<std::string, MapPtr>::iterator it =
        maps_.find(level.rawmapname);

    if(it == maps_.end())
    {
        G_LogPrintf("MapData::UpdateLastPlayed-ERROR: couldn't find map %s\n", level.rawmapname);
        return;
    }

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db_,
        "UPDATE maps SET last_played=?, seconds_played=? WHERE map=?;",
        -1, &stmt, 0);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing updateLastPlayed statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return;
    }

    int index = 1;
    rc = sqlite3_bind_int(stmt, index++, currentTime);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Failed to bind last played to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return;
    }

    rc = sqlite3_bind_int(stmt, index++, it->second->played + currentTime - mapLoadTime_);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Failed to bind seconds played to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return;
    }

    rc = sqlite3_bind_text(stmt, index++, level.rawmapname, strlen(level.rawmapname), SQLITE_STATIC);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Failed to bind map name to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return;
    }

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE)
    {
        G_LogPrintf("Failed to execute update last played statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return;
    }
}

MapData::Map::Map(): lastPlayed(0), played(0), isOnServer(false)
{

}