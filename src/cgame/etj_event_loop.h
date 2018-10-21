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

namespace ETJump
{
	enum class TaskPriorities {
		Default,
		Immediate
	};

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
		int schedule(function<void()> fn, int delay, TaskPriorities priority = TaskPriorities::Default);
		int schedulePersistent(function<void()> fn, int delay, TaskPriorities priority = TaskPriorities::Default);
		bool unschedule(int taskId);
		void shutdown();
		bool hasPendingEvents();
		int pendingEventsCount();
		void execute(int taskId); // nasty little helper to execute non persistent tasks beforehand
	private:
		void _processEvents();
		int _scheduleEvent(const Task &task, TaskPriorities priority);
		void _cleanUpEvents();
		void _iterateEvents(function<void(Task &task)> fn);
		void _removeEventsIf(function<bool(Task &task)> fn);
		Task* _findTask(int taskId);
		int64_t _getNow();
	};
}