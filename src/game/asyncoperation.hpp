#ifndef ASYNC_OPERATION_HH
#define ASYNC_OPERATION_HH

class AsyncOperation {
public:
    AsyncOperation()
    {
        
    }
    virtual ~AsyncOperation()
    {
        
    }
    virtual void Execute() = 0;
    static void *Thread(void *args);
    
};

inline void* AsyncOperation::Thread(void* args)
{
    AsyncOperation *object = static_cast<AsyncOperation*>(args);
    object->Execute();
    pthread_exit(NULL);
    return NULL;
}
#endif 