#ifndef G_MAPDATA_H
#define G_MAPDATA_H

// Used to store data about maps
// Info to store:
// When the data collection started
// When was map last played
// How many times has it been played
// How many minutes altogether

#include <vector>
#include <string>
#include <boost/algorithm/string/case_conv.hpp>
#include <sqlite3pp.h>
using std::string;
using std::vector;

extern "C" {
#include "g_local.h"
}

struct MapInfo {
    MapInfo();
    MapInfo(const string& mapName_, int lastPlayed_,
        int timesPlayed_, int minutesPlayed_);
    bool operator<(const MapInfo& rhs);
    bool operator<(const string& rhs);
    string mapName;
    int lastPlayed;
    int timesPlayed;
    int minutesPlayed;
};

class MapData {
public:
    MapData();
    ~MapData();
    // Called on G_InitGame(). Opens connections to database, caches maps
    void init();

    // returns info about the map that matches mapName. if multiple matches,
    // returns the first match
    MapInfo mapInfo(const string& mapName) const;

    // updates maps last played, times played(+1) and minutes played to new values.
    void updateMap(const string& mapName, int lastPlayed, int minutesPlayed);

    // Prints a list of all maps on the server to ent
    void printMapList(gentity_t *ent, int columns) const;

    // Prints the n most played maps on the server to ent
    void printMostPlayedMaps(gentity_t *ent, int count) const;

    // Prints the n least played maps on the server to ent
    void printLeastPlayedMaps(gentity_t *ent, int count) const;
private:
    // sqlite database handle
    sqlite3pp::database db_;

    // map list, sorted by map names
    vector<MapInfo> mapList;
    vector<MapInfo*> mostPlayedList;

    // opens and if does not exist creates a database for maps
    void openDatabase();

    // caches maps to datastructure. most played searches etc. can now be done without
    // accessing sqlite
    void cacheMaps();

    // Reads map data from sqlite db and inserts any new maps that can't yet be found on it
    void readMapData();

    // inserts a new map to database
    void insertMap(const string& mapName);

    // Returns an iterator to mapName, must be exact map name
    vector<MapInfo>::iterator findMap(const string& mapName);
};

extern MapData mapData;

#endif