#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <condition_variable>
#include <atomic>
#include <thread>

#include "etj_utilities.h"

namespace ETJump {

template <typename T>
using opt = Utilities::Optional<T>;

class SynchronizationContext {
public:
  class ResultBase {
  };

  using TaskFn = std::function<std::unique_ptr<ResultBase>()>;
  using CallbackFn = std::function<void(std::unique_ptr<ResultBase>)>;
  using ErrorFn = std::function<void(std::runtime_error)>;

  void startWorkerThreads(unsigned numThreads);
  void stopWorkerThreads();
  void postTask(TaskFn task,
                CallbackFn callback,
                ErrorFn errorCallback);
  void processCompletedTasks();

private:
  struct Operation {
    enum class Status {
      Incomplete,
      Complete,
      Error
    };

    Status status;
    TaskFn task;
    CallbackFn callback;
    ErrorFn errorCallback;
    opt<std::unique_ptr<ResultBase>> result;
    opt<std::runtime_error> error;

    Operation() = delete;

    explicit Operation(TaskFn task, CallbackFn callback, ErrorFn errorCallback)
      : status(Status::Incomplete), task(task), callback(callback),
        errorCallback(errorCallback),
        result(opt<std::unique_ptr<ResultBase>>()),
        error(std::runtime_error("")) {
    }
  };

  void worker();

  std::mutex _incompletedMutex;
  std::queue<std::unique_ptr<Operation>> _incompleted;
  std::mutex _completedMutex;
  std::queue<std::unique_ptr<Operation>> _completed;
  std::condition_variable _workAvailable;
  std::atomic<bool> _running;
  std::vector<std::thread> _threads;
};
}
