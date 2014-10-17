#ifndef OPERATION_QUEUE_HH
#define OPERATION_QUEUE_HH

class OperationQueue
{
public:
    // Initializes the mutexes
    void Init();
    // Destroys the mutexes
    void Shutdown();
    // Executes each queued operation if the mutexes aren't locked
    bool ExecuteQueuedOperations();
private:

};

#endif