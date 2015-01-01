#ifndef TIMERUN_HPP
#define TIMERUN_HPP

class Timerun {
public:
    // This is called when start timer is spawned.
    // The name is added to a list of start timer names.
    void StartTimerSpawned(const char *name);

    // This is called when an end timer is spawned.
    // The name is added to a list of end timer names.
    void EndTimerSpawned(const char *name);

    // This is called once all entities have been spawned. Checks that
    // a matching start timer is found for every end timer.
    void EntitiesSpawned();
private:
};

#endif