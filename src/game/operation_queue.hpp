#ifndef OPERATION_QUEUE_HH
#define OPERATION_QUEUE_HH

#include <string>
#include <vector>
#ifdef _WIN32
#define PTW32_STATIC_LIB
#endif
#include <pthread.h>
#include <boost/shared_ptr.hpp>

/**
 * This class is used to execute operations in the main thread after
 * a completed operation in another thread.
 **/
class OperationQueue
{
public:
	class Operation
	{
public:
		Operation();
		virtual ~Operation();
		virtual void Execute() = 0;
	};

	// Initializes the mutexes
	void Init();
	// Destroys the mutexes
	void Shutdown();
	// Executes each queued operation if the mutexes aren't locked
	bool ExecuteQueuedOperations();
	// Adds a new operation to the queue
	bool AddNewQueuedOperation(boost::shared_ptr<Operation> op);
private:
	pthread_mutex_t                            queueMutex;
	std::vector<boost::shared_ptr<Operation> > operations_;
};

#endif
