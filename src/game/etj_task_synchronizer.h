#pragma once
#include <vector>
#include <memory>

namespace ETJump
{
	class AbstractTask;

	// synchronizes any async call callback results with the main
	// thread
	class TaskSynchronizer
	{
	public:
		TaskSynchronizer();
		~TaskSynchronizer();
		void checkTasks();
		void addTask(std::unique_ptr<AbstractTask> task);
	private:
		std::vector<std::unique_ptr<AbstractTask>> _tasks;
	};
}
