#ifndef TIMERUN_HPP
#define TIMERUN_HPP

struct gentity_s;
typedef gentity_s gentity_t;

class Timerun {
public:
    // This is called when entity activates the target_starttimer
    void StartTimer(const char *runName, gentity_t *ent);

    // This is called when the entity activates the target_stoptimer
    void StopTimer(const char *runName, gentity_t *ent);
private:
};

#endif