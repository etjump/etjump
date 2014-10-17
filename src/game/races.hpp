#ifndef RACES_HH
#define RACES_HH

#include <string>
#include <vector>
#include "../json/json-forwards.h"

struct gentity_s;
typedef gentity_s gentity_t;
typedef float vec_t;
typedef vec_t vec3_t[3];
class Races
{
public:
    struct Settings
    {
        Settings() : name("no name"), map(""), creator(""), date(0), saveLimit(-1)
        {
            
        }
        // Name of the route
        std::string name;

        // Name of the map
        std::string map;
        
        // Name of the user who created the route
        std::string creator;

        // Date the route was created
        int date;

        // -1 no limit 0 no save >0 x saves
        int saveLimit;

        
        
    };
    static const unsigned MAX_CHECKPOINTS = 20;
    struct Race
    {
        std::string name;
        std::string map;
        std::string creator;
        unsigned long date;
        std::string start;
        std::string end;
        std::vector<std::string> checkpoints;
    };

    void Init();
    void Shutdown();

    Races();
    ~Races();
    

    std::string GetMessage() const;

    bool CreateStart(vec3_t origin, vec3_t angles, bool onlyReplaceStart);
    bool CreateEnd(vec3_t origin, vec3_t angles, vec3_t dimensions);
    bool CreateCheckpoint(vec3_t origin, vec3_t angles, vec3_t dimensions);
    void ClearRoute();
    bool TeleportPlayerToStart(gentity_t *player);
    bool UndoLastCheckpoint();

    bool StartRace(gentity_t *ent);
    void StopRace();
    void DesignMode(bool state);
    bool SetSettings(const std::string& name, const std::string& map, const std::string& creator,
        int date, int saveLimit);
    bool Save(const std::string& name, gentity_t *ent);
    bool Load(const std::string& name, gentity_t *ent);
private:
    
    // Think functions for end/cp
    static void EndThink(gentity_t *self);
    static void CheckpointThink(gentity_t *self);

    static bool ClientIsRacing(gentity_t *player);
    static bool CheckpointVisited(gentity_t *player, unsigned num);

    gentity_t *start_;
    gentity_t *end_;
    gentity_t *checkpoints_[MAX_CHECKPOINTS];
    unsigned numCheckpoints_;
    std::string message_;
    bool designMode_;
    Settings raceSettings_;
};

#endif // RACES_HH