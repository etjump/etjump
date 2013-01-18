#include <boost/format.hpp>
#include "g_mapdata.h"
#include "g_utilities.h"

MapData::MapData() {
    
}

MapData::~MapData() {
    db_.disconnect();
}

MapInfo::MapInfo() {
    lastPlayed = 0;
    timesPlayed = 0;
    minutesPlayed = 0;
}

MapInfo::MapInfo(const string& mapName_, 
                 int lastPlayed_,
                 int timesPlayed_, 
                 int minutesPlayed_)
{
    mapName = mapName_;
    lastPlayed = lastPlayed_;
    timesPlayed = timesPlayed_;
    minutesPlayed = minutesPlayed_;
}

bool MapInfo::operator<(const MapInfo& rhs) {
    return (this->mapName < rhs.mapName);
}

bool MapInfo::operator<(const string& rhs) {
    return (this->mapName < rhs);
}

// Called on G_InitGame(). Opens connections to database, caches maps
void MapData::init() {
    cacheMaps();
    openDatabase();
    readMapData();
}

// returns info about the map that matches mapName. if multiple matches,
    // returns the first match
MapInfo MapData::mapInfo(const string& mapName) const {
    MapInfo minfo;

    return minfo;
}

// updates maps last played, times played(+1) and minutes played to new values.
void MapData::updateMap(const string& mapName, int lastPlayed, int minutesPlayed) {

    vector<MapInfo>::iterator mapIterator = findMap(mapName);

    boost::format updateString =
        boost::format("UPDATE maps SET lastPlayed='%1%', timesPlayed='%2%', minutesPlayed='%3%' WHERE MAP='%4%';");
    updateString % (lastPlayed) % (mapIterator->timesPlayed + 1) % (mapIterator->minutesPlayed + minutesPlayed) % mapName;

    try {

        sqlite3pp::command cmd(db_, updateString.str().c_str());
        cmd.execute();

    } catch( sqlite3pp::database_error& e ) {
        LogPrintln("DATABASE ERROR WHILE UPDATING MAP DATA: " + string(e.what()));
    }
}

// Prints a list of all maps on the server to ent
void MapData::printMapList(gentity_t *ent, int columns) const {
    BeginBufferPrint();

    vector<MapInfo>::const_iterator it = mapList.begin();

    int count = 0;
    while(it != mapList.end()) {

        if(count % columns == 0 && count != 0) {
            BufferPrint(ent, "\n");
        }

        BufferPrint(ent, va("%-27s ", it->mapName.c_str()));

        it++;
        count++;
    }

    FinishBufferPrint(ent);
}

string timeToString(int time) {
    time_t t;
    tm *lt;
    char date[32];

    t = time;
    lt = localtime(&t);
    strftime(date, sizeof(date), "%d/%m/%y %H:%M:%S", lt);

    return date;
}

// Most played maps are:
// Rank Map                    Played       Time         Last played       
// 4    30           16/01/13 18:32:19
// 4|30|
string mostPlayedFormattedOutput(int rank, vector<MapInfo*>::const_iterator minfo) {

    int hours = 0;
    int minutes = 0;

    string hoursPlayed;
    string timesPlayed; 
    string lastPlayed;

    boost::format formatted_output =
                boost::format("%|1$-4| %|2$-22| %|3$-12| %|4$-12| %|5$-17| \n");

    try {
        // Format hours played
        hours = (*minfo)->minutesPlayed / 60;
        minutes = (*minfo)->minutesPlayed - hours*60;
        hoursPlayed = IntToString(hours)+"h "+IntToString(minutes)+"min";

        // Format times played
        if((*minfo)->timesPlayed == 1) {
            timesPlayed = IntToString((*minfo)->timesPlayed) + " time";
        } 
        else if((*minfo)->timesPlayed == 0) {
            timesPlayed = "never";
        }
        else {
            timesPlayed = IntToString((*minfo)->timesPlayed) + " times";
        }

        // Format last played
        if((*minfo)->lastPlayed == 0) {
            lastPlayed = "never";
        } else {
            lastPlayed = timeToString((*minfo)->lastPlayed);
        }

        formatted_output % rank % (*minfo)->mapName % timesPlayed % hoursPlayed % lastPlayed;
    }
    catch(boost::io::format_error exc) {
        LogPrintln(exc.what());
        return "Error while formatting string.";
    }
    return formatted_output.str();
}

string mostPlayedFormattedOutput(int rank, vector<MapInfo*>::const_reverse_iterator minfo) {

    int hours = 0;
    int minutes = 0;

    string hoursPlayed;
    string timesPlayed; 
    string lastPlayed;

    boost::format formatted_output =
                boost::format("%|1$-4| %|2$-22| %|3$-12| %|4$-12| %|5$-17| \n");

    try {
        // Format hours played
        hours = (*minfo)->minutesPlayed / 60;
        minutes = (*minfo)->minutesPlayed - hours*60;
        hoursPlayed = IntToString(hours)+"h "+IntToString(minutes)+"min";

        // Format times played
        if((*minfo)->timesPlayed == 1) {
            timesPlayed = IntToString((*minfo)->timesPlayed) + " time";
        } 
        else if((*minfo)->timesPlayed == 0) {
            timesPlayed = "never";
        }
        else {
            timesPlayed = IntToString((*minfo)->timesPlayed) + " times";
        }

        // Format last played
        if((*minfo)->lastPlayed == 0) {
            lastPlayed = "never";
        } else {
            lastPlayed = timeToString((*minfo)->lastPlayed);
        }

        formatted_output % rank % (*minfo)->mapName % timesPlayed % hoursPlayed % lastPlayed;
    }
    catch(boost::io::format_error exc) {
        LogPrintln(exc.what());
        return "Error while formatting string.";
    }
    return formatted_output.str();
}

static const string MORE_INFO = "^7check console for more information.";
static const string LAYOUT = "^gRank Map                    Played       Time         Last played       ";
void MapData::printMostPlayedMaps(gentity_t *ent, int count) const {
    vector<MapInfo*>::const_iterator it = mostPlayedList.begin();

    if(ent) {
        ChatPrintTo(ent, "^3!mostplayed:^7 " + MORE_INFO);
    }

    PrintTo(ent, LAYOUT);

    int rank = 1;

    BeginBufferPrint();

    while(rank <= count && it != mostPlayedList.end()) {
        BufferPrint(ent, mostPlayedFormattedOutput(rank, it));
        it++;
        rank++;
    }

    FinishBufferPrintNoNewline(ent);
}

// Prints the n least played maps on the server to ent
void MapData::printLeastPlayedMaps(gentity_t *ent, int count) const {
    vector<MapInfo*>::const_reverse_iterator it = mostPlayedList.rbegin();

    if(ent) {
        ChatPrintTo(ent, "^3!leastplayed:^7 " + MORE_INFO);
    }

    PrintTo(ent, LAYOUT);

    int rank = 1;

    BeginBufferPrint();

    while(rank <= count && it != mostPlayedList.rend()) {
        BufferPrint(ent, mostPlayedFormattedOutput(rank, it));
        it++;
        rank++;
    }

    FinishBufferPrintNoNewline(ent);
}

// opens and if does not exist creates a database for maps
void MapData::openDatabase() {
    string fileName;
    char modFolder[MAX_TOKEN_CHARS];

    trap_Cvar_VariableStringBuffer("fs_game", modFolder, sizeof(modFolder));

    if(strlen(modFolder)) {
        fileName = string(modFolder) + "/" + string("mapdata.db");
    } else {
        fileName = "etjump/mapdata.db";
    }

    try {
        db_.connect(fileName.c_str());
        sqlite3pp::command cmd(db_, "CREATE TABLE IF NOT EXISTS maps(id INTEGER PRIMARY KEY AUTOINCREMENT, map varchar(128), lastPlayed INTEGER, timesPlayed INTEGER, minutesPlayed INTEGER);");
        cmd.execute();
    }

    catch( sqlite3pp::database_error& e ) {
        LogPrintln("DATABASE ERROR: " + string(e.what()));
    }

}

// caches maps to datastructure. -most played- searches etc. can now be done without
// accessing sqlite
void MapData::cacheMaps() {
    int numdirs = 0;
    char dirlist[8192];
	char* dirptr;
	int i;
	int dirlen;
    numdirs = trap_FS_GetFileList("maps", ".bsp", dirlist, sizeof(dirlist));
    mapList.clear();
    mostPlayedList.clear();

	dirptr = dirlist;
	for(i = 0; i < numdirs; i++, dirptr += dirlen+1) {
		dirlen = strlen(dirptr);
		if(strlen(dirptr) > 4)
			dirptr[strlen(dirptr)-4] = '\0';
        if(dirptr) {
            string lowercase = dirptr;
            boost::to_lower(lowercase);
            MapInfo minfo = MapInfo(lowercase, 0, 0, 0);
            mapList.push_back(minfo);
        }
	}
    std::sort(mapList.begin(), mapList.end());
}

bool mostPlayedComparator(const MapInfo *lhs, const MapInfo* rhs) {
    return (lhs->minutesPlayed > rhs->minutesPlayed);
}

// Reads map data from sqlite db and inserts any new maps that can't yet be found on it
void MapData::readMapData() {
    try {
        sqlite3pp::query query(db_, "SELECT map, lastPlayed, timesPlayed, minutesPlayed FROM maps ORDER BY map ASC;");

        vector<MapInfo>::iterator mapInfoIterator = mapList.begin();
        sqlite3pp::query::iterator queryIterator = query.begin();

        while(mapInfoIterator != mapList.end()) {

            if(queryIterator == query.end()) {
                insertMap(mapInfoIterator->mapName);
                mapInfoIterator->lastPlayed = 0;
                mapInfoIterator->timesPlayed = 0;
                mapInfoIterator->minutesPlayed = 0;
            }

            else {

                int lastPlayed = 0;
                int timesPlayed = 0;
                int minutesPlayed = 0;
                const char *mapNamePtr = 0;

                boost::tie(mapNamePtr, lastPlayed, timesPlayed, minutesPlayed)
                    = queryIterator->get_columns<const char*, int, int, int>(0, 1, 2, 3);

                // If we found a matching map, just update it's data
                if(mapInfoIterator->mapName == mapNamePtr) {
                    mapInfoIterator->lastPlayed = lastPlayed;
                    mapInfoIterator->timesPlayed = timesPlayed;
                    mapInfoIterator->minutesPlayed = minutesPlayed;
                    queryIterator++;
                }
                // If we couldn't find it on map database, we need to add it there
                // Dont need to increment queryIterator
                else {
                    insertMap(mapInfoIterator->mapName);
                    mapInfoIterator->lastPlayed = 0;
                    mapInfoIterator->timesPlayed = 0;
                    mapInfoIterator->minutesPlayed = 0;
                }
            }

            mapInfoIterator++;
        }
    }

    catch( sqlite3pp::database_error& e ) {
        LogPrintln("DATABASE ERROR: " + string(e.what()));
    }

    vector<MapInfo>::iterator mapInfoIterator = mapList.begin();
    while(mapInfoIterator != mapList.end()) {
        mostPlayedList.push_back(&(*mapInfoIterator));
        mapInfoIterator++;
    }
    std::sort(mostPlayedList.begin(), mostPlayedList.end(), mostPlayedComparator);
}

// inserts a new map to database
void MapData::insertMap(const string& mapName) {
try {
    sqlite3pp::command cmd(db_, "INSERT INTO maps (map, lastPlayed,\
                                timesPlayed, minutesPlayed)\
                                VALUES (?1, ?2, ?3, ?4);");
    cmd.bind(1, mapName.c_str());
    cmd.bind(2, 0);
    cmd.bind(3, 0);
    cmd.bind(4, 0);
    cmd.execute();
    }

    catch( sqlite3pp::database_error& e ) {
        LogPrintln("DATABASE ERROR WHILE INSERTING NEW MAP: " + string(e.what()));
        return;
    }
}

// Returns an iterator to mapName, must be exact map name
vector<MapInfo>::iterator MapData::findMap(const string& mapName) {
    return std::lower_bound(mapList.begin(),
        mapList.end(), mapName);
}