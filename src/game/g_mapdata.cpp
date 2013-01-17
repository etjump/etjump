#include <boost/format.hpp>
#include "g_mapdata.h"
#include "g_utilities.h"

MapData::MapData() {
    
}

MapData::~MapData() {
    db_.disconnect();
}

void MapData::cacheMapNames() {
    int numdirs = 0;
    char dirlist[8192];
	char* dirptr;
	int i;
	int dirlen;
    numdirs = trap_FS_GetFileList("maps", ".bsp", dirlist, sizeof(dirlist));

	dirptr = dirlist;
	for(i = 0; i < numdirs; i++, dirptr += dirlen+1) {
		dirlen = strlen(dirptr);
		if(strlen(dirptr) > 4)
			dirptr[strlen(dirptr)-4] = '\0';
        if(dirptr) {
            string lowercase = dirptr;
            boost::to_lower(lowercase);
            mapList.push_back(lowercase);
        }
	}
}

void MapData::openDatabase() {
    // Open up DB & create table if it does not exist

    string filename;
    char mod_folder[MAX_TOKEN_CHARS];

    trap_Cvar_VariableStringBuffer("fs_game", mod_folder, sizeof(mod_folder));

    if(strlen(mod_folder)) {
        filename = string(mod_folder) + "/" + string("mapdata.db");
    } else {
        filename = "etjump/" + string("mapdata.db");
    }

    try {
        db_.connect(filename.c_str());

        sqlite3pp::command cmd(db_, "CREATE TABLE IF NOT EXISTS maps(id INTEGER PRIMARY KEY AUTOINCREMENT, map varchar(128), lastplayed INTEGER, timesplayed INTEGER, minutesplayed INTEGER);");
        cmd.execute();
    }

    catch( sqlite3pp::database_error& e ) {
        LogPrintln("DATABASE ERROR: " + string(e.what()));
    }
}

bool MapData::init() {
    cacheMapNames();
    openDatabase();
    updateMostPlayed();
    return true;
}

void MapData::insertNewMap(const string& mapName, int lastPlayed, int timesPlayed, int minutesPlayed) {
    try {
        sqlite3pp::command cmd(db_, "INSERT INTO maps (map, lastplayed,\
                                    timesplayed, minutesplayed)\
                                    VALUES (?1, ?2, ?3, ?4);");
            cmd.bind(1, mapName.c_str());
            cmd.bind(2, lastPlayed);
            cmd.bind(3, timesPlayed);
            cmd.bind(4, minutesPlayed);
            cmd.execute();
    }

    catch( sqlite3pp::database_error& e ) {
        LogPrintln("DATABASE ERROR WHILE INSERTING NEW MAP: " + string(e.what()));
        return;
    }
}

void MapData::updateMap(const string& mapName, int lastPlayed, int timesPlayed, int minutesPlayed) {
    try {
        boost::format update_string =
            boost::format("UPDATE maps SET lastplayed='%1%',\
                          timesplayed='%2%',\
                          minutesplayed='%3%'\
                          WHERE map='%4%';");
        update_string % lastPlayed % timesPlayed % minutesPlayed % mapName;
            
        sqlite3pp::command cmd(db_, update_string.str().c_str());
        cmd.execute();
    }

    catch( sqlite3pp::database_error& e ) {
        LogPrintln("DATABASE ERROR WHILE UPDATING MAP DATA: " + string(e.what()));
        return;
    }
}

void MapData::update(const string& mapName, int lastPlayed, int timesPlayed, int minutesPlayed) {
    // Check if the map exists
    // SELECT timesplayed, minutesplayed FROM maps WHERE map = 'mapName';
    boost::format query_string = 
        boost::format("SELECT timesplayed, minutesplayed FROM maps WHERE map ='%1%';");
    query_string % mapName;

    try {
        sqlite3pp::query query(db_, query_string.str().c_str());
        sqlite3pp::query::iterator it = query.begin();

        // Found a matching map, just update it
        if(it != query.end()) {
            int previous_timesPlayed = 0;
            int previous_minutesPlayed = 0;
            boost::tie(previous_timesPlayed, previous_minutesPlayed) =
                (*it).get_columns<int, int>(0, 1);
            updateMap(mapName, lastPlayed, 
                timesPlayed + previous_timesPlayed, 
                minutesPlayed + previous_minutesPlayed);
        }
        // Didn't find a matching map, insert it.
        else {
            insertNewMap(mapName, lastPlayed, timesPlayed, minutesPlayed);
        }
    }

    catch( sqlite3pp::database_error& e ) {
        LogPrintln("DATABASE ERROR: " + string(e.what()));
        return;
    }
}

void MapData::updateMapDatabase() {
    /* Fucks everything up
    vector<string>::const_iterator it = mapList.begin();
    while(it != mapList.end()) {
        try {
            sqlite3pp::command cmd(db_, "INSERT INTO maps (map, lastplayed, timesplayed, minutesplayed) VALUES (?1, ?2, ?3, ?4);");
            cmd.bind(1, (*it).c_str());
            cmd.bind(2, 0);
            cmd.bind(3, 0);
            cmd.bind(4, 0);
            cmd.execute();
            it++;
        }

        catch ( ... ) {
            return;
        }
    }
    */
}

MapInfo::MapInfo() {
    timesPlayed = 0;
    minutesPlayed = 0;
    lastPlayed = 0;
    mapName.clear();
}

MapInfo::MapInfo(const string& mapName_, int lastPlayed_,
        int timesPlayed_, int minutesPlayed_) {
    timesPlayed = timesPlayed_;
    minutesPlayed = minutesPlayed_;
    mapName = mapName_;
    lastPlayed = lastPlayed_;
}

const string& MapData::findMap(const string& mapName) {
    vector<string>::const_iterator it = mapList.begin();

    while(it != mapList.end()) {
        if(it->find(mapName) != string::npos) {
            return *it;
        }
        it++;
    }
    return "";
}

MapInfo MapData::mapInfo(const string& mapName) {
    MapInfo minfo;

    string fullName = findMap(mapName);

    try {
        boost::format query_string("SELECT lastplayed, timesplayed, minutesplayed FROM maps WHERE map='%1%';");
        query_string % fullName;
        sqlite3pp::query query(db_, query_string.str().c_str());
        
        sqlite3pp::query::iterator it = query.begin();

        if(it != query.end()) {
            boost::tie(minfo.lastPlayed, minfo.timesPlayed, minfo.minutesPlayed)
                = (*it).get_columns<int, int, int>(0, 1, 2);
            minfo.mapName = fullName;
        }
        // Didn't find anything
        else {
            minfo.mapName = "MAP NOT FOUND";
        }
    }

    catch( sqlite3pp::database_error& e ) {
        LogPrintln("DATABASE ERROR: " + string(e.what()));
        return minfo;
    }
    
    return minfo;
}

void MapData::updateMostPlayed() {
    mostPlayedMaps.clear();
    try {
        const char *mapNamePtr = 0;
        MapInfo minfo;
        sqlite3pp::query query(db_, "SELECT map, lastplayed, timesplayed, minutesplayed FROM maps ORDER BY minutesplayed DESC LIMIT 10;");

        sqlite3pp::query::iterator it = query.begin();

        while(it != query.end()) {
            boost::tie(mapNamePtr, minfo.lastPlayed, minfo.timesPlayed, minfo.minutesPlayed)
                = (*it).get_columns<const char*, int, int, int>(0, 1, 2, 3);
            if(mapNamePtr) {
                minfo.mapName = mapNamePtr;
            }
            mostPlayedMaps.push_back(minfo);
            it++;
        }
    }

    catch( sqlite3pp::database_error& e ) {
        LogPrintln("DATABASE ERROR: " + string(e.what()));
        return;
    }

}

const vector<MapInfo> *MapData::mostPlayed() {
    return &mostPlayedMaps;
}