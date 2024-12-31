/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <condition_variable>
#include <atomic>
#include <thread>

#include "etj_shared.h"
#include "etj_utilities.h"

namespace ETJump {
class SynchronizationContext {
public:
  class ResultBase {
  public:
    virtual ~ResultBase() = default;
  };

  using TaskFn = std::function<std::unique_ptr<ResultBase>()>;
  using CallbackFn = std::function<void(std::unique_ptr<ResultBase>)>;
  using ErrorFn = std::function<void(std::runtime_error)>;

  void startWorkerThreads(unsigned numThreads);
  void stopWorkerThreads();
  void postTask(TaskFn task, CallbackFn callback, ErrorFn errorCallback);
  void processCompletedTasks();

private:
  struct Operation {
    enum class Status { Incomplete, Complete, Error };

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
          error(std::runtime_error("")) {}
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
} // namespace ETJump
