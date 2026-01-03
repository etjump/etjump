/*
 * MIT License
 *
 * Copyright (c) 2026 ETJump team <zero@etjump.com>
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

/*
        Single threaded(non thread safe) ordered task scheduler.
*/
#pragma once

#ifdef min
  #undef min
#endif
#ifdef max
  #undef max
#endif

#include <functional>
#include <vector>
#include <cstdint>

using std::function;
using std::vector;

namespace ETJump {
enum class TaskPriorities { Default, Immediate };

class EventLoop {
  struct Task {
    function<void()> fn;
    int id;
    int delay;
    int64_t end;
    bool persistent;
    bool deprecated;
  };
  vector<Task> ordinaryTasks;
  vector<Task> importantTasks;
  int eventCounter = 0;
  bool isExecutingEvents = false;

public:
  EventLoop() {};
  ~EventLoop() {};
  void run();
  int schedule(function<void()> fn, int delay,
               TaskPriorities priority = TaskPriorities::Default);
  int schedulePersistent(function<void()> fn, int delay,
                         TaskPriorities priority = TaskPriorities::Default);
  bool unschedule(int taskId);
  void shutdown();
  bool hasPendingEvents();
  int pendingEventsCount();
  void execute(int taskId); // nasty little helper to execute non
                            // persistent tasks beforehand
private:
  void processEvents();
  int scheduleEvent(const Task &task, TaskPriorities priority);
  void cleanUpEvents();
  void iterateEvents(function<void(Task &task)> fn);
  void removeEventsIf(function<bool(Task &task)> fn);
  void removeEvents(function<void(Task &task)> fn);
  Task *findTask(int taskId);
  int64_t getNow();
};
} // namespace ETJump
