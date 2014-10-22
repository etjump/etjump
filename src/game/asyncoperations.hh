#ifndef ASYNC_OPERATIONS_HH
#define ASYNC_OPERATIONS_HH

#include "asyncoperation.hpp"
#include "races.hpp"

class AsyncSaveRace : public AsyncOperation
{
public:
    AsyncSaveRace(Races::Race val, gentity_t *ent) : data_(val), ent_(ent)
    {
        
    }

private:
    Races::Race data_;
    gentity_t *ent_;
    void Execute();
};

class AsyncLoadRace : public AsyncOperation
{
public:
    AsyncLoadRace(Races *races, std::string name, gentity_t *ent, OperationQueue *queue) : races_(races), name_(name), ent_(ent), queue_(queue)
    {

    }

private:
    Races *races_;
    std::string name_;
    gentity_t *ent_;
    OperationQueue *queue_;
    void Execute();
};

#endif