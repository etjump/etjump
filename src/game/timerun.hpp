#ifndef TIMERUN_HPP
#define TIMERUN_HPP

#include <string>

struct gentity_s;
typedef gentity_s gentity_t;

class Timerun {
public:
    Timerun();
    ~Timerun();

    static const int MAX_PLAYERS = 64;

    struct Player
    {
        Player() : racing(false), runName(""), raceStartTime(0)
        {
            
        }
        bool racing;
        std::string runName;
        int raceStartTime;
    };

    // Initializes the sqlite database
    void Initialize();

    void Shutdown();

    // This is called when entity activates the target_starttimer
    void StartTimer(const char *runName, gentity_t *ent);

    // This is called when the entity activates the target_stoptimer
    void StopTimer(const char *runName, gentity_t *ent);
private:
    Player players_[MAX_PLAYERS];
};

#endif  