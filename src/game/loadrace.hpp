#ifndef LOAD_RACE_HH
#define LOAD_RACE_HH

#include "operationqueue.hpp"
#include "races.hpp"

struct gentity_s;
typedef gentity_s gentity_t;

class LoadRace : public OperationQueue::Operation
{
public:
    LoadRace(Races *races, Races::Race race, gentity_t *ent);
    ~LoadRace();
    void Execute();
private:
    Races *races_;
    Races::Race race_;
    gentity_t *ent_;
};

#endif 