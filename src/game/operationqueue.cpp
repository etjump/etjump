#include "operationqueue.hpp"
#include "g_local.hpp"
#include <pthread.h>

void OperationQueue::Init()
{
    pthread_mutex_init(&queueMutex, NULL);
}

void OperationQueue::Shutdown()
{
    pthread_mutex_destroy(&queueMutex);
}

bool OperationQueue::ExecuteQueuedOperations()
{
    // If we can't lock the mutex we'll try again next frame
    if (pthread_mutex_trylock(&queueMutex))
    {
        G_LogPrintf("Mutex is currently locked. Let's try again next frame.\n");
        return false;
    }

    if (operations_.size() > 0)
    {
        G_LogPrintf("Executing %d operations from the operation queue\n", operations_.size());

        for (unsigned i = 0; i < operations_.size(); i++)
        {
            operations_[i]->Execute();
        }

        operations_.clear();
    }    

    pthread_mutex_unlock(&queueMutex);
    return true;
}

bool OperationQueue::AddNewQueuedOperation(boost::shared_ptr<Operation> op)
{
    // Let's wait until we get to lock the mutex
    pthread_mutex_lock(&queueMutex);


    operations_.push_back(op);

    pthread_mutex_unlock(&queueMutex);
    return true;
}

OperationQueue::Operation::Operation()
{
}

OperationQueue::Operation::~Operation()
{
}
