//
// Created by Jussi on 5.4.2015.
//

#ifndef ETJUMP_TIMERUN_H
#define ETJUMP_TIMERUN_H

#include <vector>
#include <memory>
#include <array>
#include <map>

class Timerun {
public:
    /**
     * A single record on a single run
     */
    struct Record {
        Record(): id(-1), time(0), date(0), userId(0) { }
        int id;
        int time;
        int date;
        int userId;
        std::string map;
        std::string run;
        std::string playerName;
    };

    /**
     * A connected client
     */
    struct Player {
        Player(int userId): racing(false),
                  userId(userId),
                  name(""),
                  currentRunName(""),
                  runStartTime(0),
                  completionTime(0)
        {

        }
        int userId;
        bool racing;
        std::string name;
        std::string currentRunName;
        int runStartTime;
        int completionTime;
    };

    /**
     * Initializes the database. Reads current map records to memory
     * @param database The database file name
     * @param currentMap The map currently on the server
     * @return true if successful
     */
    bool init(const std::string& database, const std::string& currentMap);

    /**
     * Initializes a user.
     * @param clientNum The user's client slot
     * @param userId The users sqlite database id
     * @return true if successful
     */
    bool clientConnect(int clientNum, int userId);

    /**
     * When a player touches the start timer this function is called.
     * Sets the player status to racing if it's not racing already.
     * @param runName The run name player is trying to start
     * @param clientNum The player client number that started the run
     * @param playerName The players current name that will be stored to db if he makes a new record
     * @param runStartTime The time when racing was started
     */
    void startTimer(const std::string& runName, int clientNum, const std::string& currentName, int raceStartTime);

    /**
     * Notifies the client and client's spectators about a run that just started
     * @param clientNum The player who started the run
     */
    void startNotify(int clientNum);

    /**
     * When a player touches the stop timer this function is called.
     * If player is racing and the end point matches the start point,
     * prints the time and saves the record. Else ignored
     * @param clientNum Player's client number
     * @param commandTime client's ps.commandTime (used to get the completion time)
     */
    void stopTimer(int clientNum, int commandTime, std::string runName);

    /**
     * Interrupts the player's current run.
     * @param clientNum The player who's run will be interrupted
     */
    void interrupt(int clientNum);

    /**
     * Prints either top 50 records from 1 run or all #1s from all runs
     * @param clientNum the player who's calling the function
     * @param map The map 
     * @param runName The run
     */
    void printRecords(int clientNum, const std::string& map, const std::string&runName);

    std::string getMessage() const
    {
        return _message;
    }
private:
    /**
     * Updates users current record. If user has no record, record will
     * be inserted
     * @param The player who's record we're checking
     * @param clientNum The client slot number of the player
     */
    void checkRecord(Player *player, int clientNum);

    /**
     * Sorts all records 
     */
    void sortRecords();

    /**
     * Finds the previous record of a player's current run
     * @param Player the player who's record we're looking for
     * @return Record * A pointer to the record
     */
    Record *findPreviousRecord(Player *player);

    /**
     * Updates the previous record if new one is faster
     * @param previousRecord The pointer to the previous record
     * @param player Pointer to the player who's record we're updating
     * @param clientNum Player's client num
     */
    void updatePreviousRecord(Record *previousRecord, Player *player, int clientNum);

    /**
     * Adds a new record
     * @param player The player who's record we're adding
     * @param clientNum player's clientNum
     */
    void addNewRecord(Player *player, int clientNum);

    /**
     * Spawns a thread that will save the record
     * @param record The record that will be added/updated
     * @param update Is the record a new one or is it getting updated
     */
    void SaveRecord(Record *record, bool update);

    /**
     * Error or other message
     */
    std::string _message;

    /**
     * Current map on server
     */
    std::string _currentMap;

    /**
     * Database file name
     */
    std::string _database;

    /**
     * List of records in current map
     */
    std::map<std::string, std::vector<std::unique_ptr<Record> > > _records;

    /**
     * Sorted status
     */
    std::map<std::string, bool> _sorted;

    /**
     * List of currently connected players (and not connected but
     * theyre nullptrs)
     */
    std::array<std::unique_ptr<Player>, 64> _players;
};


#endif //ETJUMP_TIMERUN_H
