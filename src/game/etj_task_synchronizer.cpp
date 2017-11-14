#include "etj_task_synchronizer.h"
#include "etj_task.h"

ETJump::TaskSynchronizer::TaskSynchronizer()
{
	_tasks.empty();
}

ETJump::TaskSynchronizer::~TaskSynchronizer()
{
}

void ETJump::TaskSynchronizer::checkTasks()
{
	std::vector<int> deletedTask;
	for (int i = 0, len = _tasks.size(); i < len; ++i)
	{
		if (_tasks[i]->isReady())
		{
			_tasks[i]->execute();
			deletedTask.push_back(i);
		}
	}

	std::vector<std::unique_ptr<AbstractTask>> temp;
	for (int i = 0, len = _tasks.size(); i < len; ++i)
	{
		if (find(begin(deletedTask), end(deletedTask), i) == end(deletedTask))
		{
			temp.push_back(std::move(_tasks[i]));
		}
	}
	_tasks = std::move(temp);
}

void ETJump::TaskSynchronizer::addTask(std::unique_ptr<AbstractTask> task)
{
	_tasks.push_back(std::move(task));
}
