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
    string mapName;
    int lastPlayed;
    int timesPlayed;
    int minutesPlayed;
};

class MapData {
public:
    MapData();
    ~MapData();
    // Called on initgame
    // Opens connection to DB and caches map names. Called on initgame
    bool init();
    // returns info about map
    MapInfo mapInfo(const string& mapName);
    void update(const string& mapName, int lastPlayed, int timesPlayed, int minutesPlayed);
    // Used to add all maps to database. Very expensive, don't use often
    void updateMapDatabase();
    // Updates most played maps
    void updateMostPlayed();
    // Returns a vector of 10 most played maps
    const vector<MapInfo> *mostPlayed();
private:
    sqlite3pp::database db_;
    vector<string> mapList;    
    // Updated on initgame
    vector<MapInfo> mostPlayedMaps;

    void openDatabase();
    void cacheMapNames();
    void insertNewMap(const string& mapName, int lastPlayed, int timesPlayed, int minutesPlayed);
    void updateMap(const string& mapName, int lastPlayed, int timesPlayed, int minutesPlayed);
    const string& findMap(const string& mapName);
};

extern MapData mapData;


#endif