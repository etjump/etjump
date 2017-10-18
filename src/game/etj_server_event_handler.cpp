#include "etj_server_event_handler.h"


int ETJump::ServerEventHandler::subscribeToRunFrame(std::function<void(int levelTime)> callback)
{
	auto id = nextCallbackId++;
	_runFrameCallbacks.push_back({id, callback});
	return id;
}

int ETJump::ServerEventHandler::subscribeToShutdownGame(std::function<void(int restart)> callback)
{
	auto id = nextCallbackId++;
	_runFrameCallbacks.push_back({id, callback});
	return id;
}

void ETJump::ServerEventHandler::runFrame(int levelTime)
{
	for (auto& iter : _runFrameCallbacks)
	{
		iter.callback(levelTime);
	}
}

void ETJump::ServerEventHandler::shutdownGame(int restart)
{
	for (auto& iter : _shutdownGameCallbacks)
	{
		iter.callback(restart);
	}
}
