#ifndef RACES_HH
#define RACES_HH

#include <string>

struct gentity_s;
typedef gentity_s gentity_t;
typedef float vec_t;
typedef vec_t vec3_t[3];
class Races
{
public:
    Races();
    ~Races();
    static const unsigned MAX_CHECKPOINTS = 20;

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
};

#endif // RACES_HH