#ifndef MAPDATA_H
#define MAPDATA_H
#include <map>
#include <boost/shared_ptr.hpp>
#include <sqlite3.h>
#include "leveldata.h"

class MapData
{
public:
    MapData();
    void Initialize();
    void Shutdown();

    std::string RandomMap();
    void ListMaps(gentity_t *ent);
    void PrintMapInfo(gentity_t *ent, const std::string& name);

    struct Map
    {
        Map();
        int lastPlayed;
        int played;
        bool isOnServer;
    };

    typedef boost::shared_ptr<Map> MapPtr;

private:
    bool OpenMapDatabase();
    bool CreateMapsTable();
    void LoadMapData();
    void UpdateLastPlayed();

    std::map<std::string, MapPtr> maps_; 
    sqlite3 *db_;

    int mapLoadTime_;
};

#endif // MAPDATA_H