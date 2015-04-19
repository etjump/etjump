//
// Created by Jussi on 13.4.2015.
//

#include "Worker.h"
#include "Printer.h"
#include <boost/format.hpp>

void Worker::produce(std::function<void()> task)
{
    std::unique_lock<std::mutex> lk(_mtx);

    _workQueue.push(task);

    if (_workQueue.size() == 1) {
        _conditionVariable.notify_one();
    }
}

void Worker::consume()
{
    std::unique_lock<std::mutex> lk(_mtx);

    while (_workQueue.size() == 0) {
        _conditionVariable.wait(lk);
    }

    std::function<void()> task = _workQueue.front();
    _workQueue.pop();
    lk.unlock();

    task();
}

void Worker::work()
{
    _working = true;
    std::thread t([this]() {
        ++_threadCount;
        while(_working) {
            consume();
        }
        --_threadCount;
    });
    t.detach();
}

void Worker::stop()
{
    _working = false;
}
