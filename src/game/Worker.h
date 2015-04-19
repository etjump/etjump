//
// Created by Jussi on 13.4.2015.
//

#ifndef ETJUMP_WORKER_H
#define ETJUMP_WORKER_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>

class Worker {
public:
    Worker(): _threadCount(0), _working(false) {}
    ~Worker() {
        this->stop();
    }
    /**
     * Adds a task to the queue. Blocks until work queue
     * is unlocked.
     * @param task The task to be added
     */
    void produce(std::function<void ()> task);

    /**
     * Removes a task from the queue. Blocks until the task is removed.
     * Executes the task.
     */
    void consume();

    /**
     * Completes tasks until _working is set to false
     */
    void work();

    /**
     * Stops working threads if there are any
     */
    void stop();
private:
    std::atomic<int> _threadCount;
    std::atomic<bool> _working;
    std::condition_variable _conditionVariable;
    std::queue<std::function<void ()> > _workQueue;
    std::mutex _mtx;
};


#endif //ETJUMP_WORKER_H
