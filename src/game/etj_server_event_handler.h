#pragma once
#include <vector>
#include <functional>

namespace ETJump
{
	class ServerEventHandler
	{
	public:
		struct InitGameCallback
		{
			int id;
			std::function<void(int levelTime, int randomSeed, int restart)> callback;
		};

		struct RunFrameCallback
		{
			int id;
			std::function<void(int levelTime)> callback;
		};

		struct ShutdownGameCallback
		{
			int id;
			std::function<void(int restart)> callback;
		};

		ServerEventHandler()
			: nextCallbackId(1), /*_initGameCallbacks{},*/ _runFrameCallbacks{}, _shutdownGameCallbacks{}{}
		~ServerEventHandler() {}

		//int subscribeToInitGame(std::function<void(int levelTime, int randomSeed, int restart)> callback)
		//{
		//	auto id = nextCallbackId++;
		//	_initGameCallbacks.push_back({ id, callback });
		//	return id;
		//}

		int subscribeToRunFrame(std::function<void(int levelTime)> callback);

		int subscribeToShutdownGame(std::function<void(int restart)> callback);

		//void initGame(int levelTime, int randomSeed, int restart)
		//{
		//	for (auto& iter : _initGameCallbacks)
		//	{
		//		iter.callback(levelTime, randomSeed, restart);
		//	}
		//}

		void runFrame(int levelTime);

		void shutdownGame(int restart);
	private:
		int nextCallbackId;
		//std::vector<InitGameCallback> _initGameCallbacks;
		std::vector<RunFrameCallback> _runFrameCallbacks;
		std::vector<ShutdownGameCallback> _shutdownGameCallbacks;
	};
}
