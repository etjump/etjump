#ifndef TIMERUN_HPP
#define TIMERUN_HPP

#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include "asyncoperation.hpp"
#include <ostream>


class Session;
class SaveSystem;
struct gentity_s;
typedef gentity_s gentity_t;

class Timerun {
public:
    Timerun(Session *session, SaveSystem *saveSystem);
    ~Timerun();

    static const int MAX_PLAYERS = 64;

    struct Player
    {
        Player() : racing(false), userDatabaseId(0), currentName(""), runName(""), raceStartTime(0), time(0)
        {
            
        }
        bool racing;
        int userDatabaseId;
        std::string currentName;
        std::string runName;
        int raceStartTime;
        int time;
    };

    struct Record
    {
        Record(): id(0), time(0), map(""), run(""), player(0), playerName("") {} 
        int id;
        int time;
        std::string map;

        friend std::ostream& operator<<(std::ostream& os, const Record& obj)
        {
            return os
                << "id: " << obj.id
                << " time: " << obj.time
                << " map: " << obj.map
                << " run: " << obj.run
                << " player: " << obj.player
                << " playerName: " << obj.playerName;
        }

        std::string run;
        int player;
        std::string playerName;
    };

    // Initializes the sqlite database
    void Initialize();

    void Shutdown();

    // This is called when entity activates the target_starttimer
    void StartTimer(const char *runName, gentity_t *ent);

    // This is called when the entity activates the target_stoptimer
    void StopTimer(const char *runName, gentity_t *ent);

    // Prints the records of the current map
    void PrintRecords(gentity_t *ent);
private:
    Session *session_;
    SaveSystem *saveSystem_;
    Player players_[MAX_PLAYERS];
    // The records data structure has records in the current map.
    std::map<std::string, std::vector<boost::shared_ptr<Record> > > records_;
    typedef std::map<std::string, std::vector<boost::shared_ptr<Record> > >::iterator RunIterator;
    typedef std::vector<boost::shared_ptr<Record> >::iterator RecordIterator;
    
    void InsertRecord(std::string mapName, Player *player);

    class InsertRecordOperation : public AsyncOperation
    {
    public:
        InsertRecordOperation(std::string mapName, Player player, bool update);
        ~InsertRecordOperation();
    private:
        std::string mapName_;
        Player player_;
        bool update_;
        void Execute();
    };
};

#endif  