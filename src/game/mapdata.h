#ifndef MAPDATA_H
#define MAPDATA_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "g_local.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>

using namespace boost::multi_index;

class MapData
{
public:
    struct Map_s
    {
        Map_s() : id(0), lastPlayed(0), secondsPlayed(0), mapOnServer(false)
        {
            
        }
        unsigned id;
        std::string name;
        unsigned lastPlayed;
        unsigned secondsPlayed;
        bool mapOnServer;

        const char *ToChar()
        {
            return va("%d %s %d %d", id, name.c_str(), lastPlayed, secondsPlayed);
        }

        std::string GetName() 
        {
            return name;
        }

        unsigned GetLastPlayed() 
        {
            return lastPlayed;
        }

        unsigned GetSecondsPlayed() 
        {
            return secondsPlayed;
        }
    };

    typedef boost::shared_ptr<Map_s> Map;

    typedef boost::multi_index_container<
        Map,
        indexed_by<
        ordered_unique<mem_fun<Map_s, std::string, &Map_s::GetName> >,
        ordered_non_unique<mem_fun<Map_s, unsigned, &Map_s::GetLastPlayed> >,
        ordered_non_unique<mem_fun<Map_s, unsigned, &Map_s::GetSecondsPlayed> >
        >
    > Maps;

    typedef Maps::nth_index<0>::type::iterator NameIterator;
    typedef Maps::nth_index<0>::type::const_iterator ConstNameIterator;
    typedef Maps::nth_index<0>::type::reverse_iterator RNameIterator;
    typedef Maps::nth_index<0>::type::const_reverse_iterator ConstRNameIterator;

    typedef Maps::nth_index<1>::type::iterator LastPlayedIterator;
    typedef Maps::nth_index<1>::type::const_iterator ConstLastPlayedIterator;
    typedef Maps::nth_index<1>::type::reverse_iterator RLastPlayedIterator;
    typedef Maps::nth_index<1>::type::const_reverse_iterator ConstRLastPlayedIterator;

    typedef Maps::nth_index<2>::type::iterator SecondsPlayedIterator;
    typedef Maps::nth_index<2>::type::const_iterator ConstSecondsPlayedIterator;
    typedef Maps::nth_index<2>::type::reverse_iterator RSecondsPlayedIterator;
    typedef Maps::nth_index<2>::type::const_reverse_iterator ConstRSecondsPlayedIterator;


    bool Initialize();
    void Shutdown();

    void ListMaps(gentity_t *ent);
    void ListLeastPlayed(gentity_t *ent);
    void ListMostPlayed(gentity_t *ent);
    void PrintMapInfo(gentity_t *ent, const std::string& mapName);
    std::string RandomMap();
    std::string GetMessage();
private:
    bool BindInt(sqlite3_stmt* stmt, int index, int val);
    bool BindString(sqlite3_stmt* stmt, int index, const std::string& val);
    bool PrepareStatement(char const* query, sqlite3_stmt** stmt);
    ConstNameIterator GetNameIterEnd();
    ConstLastPlayedIterator GetLPIterEnd();
    ConstSecondsPlayedIterator GetSPIterEnd();
    NameIterator GetMapByName(const std::string& name);
    LastPlayedIterator GetMapByLP(unsigned lastPlayed);
    SecondsPlayedIterator GetMapBySP(unsigned secondsPlayed);

    Maps maps_;
    std::string message_;
    sqlite3 *db_;
    time_t mapStartTime_;
};

#endif


