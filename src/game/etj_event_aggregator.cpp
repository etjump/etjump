#include "etj_event_aggregator.h"

ETJump::EventAggregator::EventAggregator(): _nextFreeHandle(0) {
}

ETJump::EventAggregator::~EventAggregator()
{
	_serverCallbacks.clear();
	_clientCallbacks.clear();
}

int ETJump::EventAggregator::subscribe(ServerEventType eventType, std::function<void(const Payload *)> callback)
{
	auto handle = _nextFreeHandle++;

	_serverCallbacks.push_back({ true, handle, eventType, callback });

	return handle;
}

int ETJump::EventAggregator::subscribe(ClientEventType eventType, std::function<void(const Payload *)> callback)
{
	auto handle = _nextFreeHandle++;

	_clientCallbacks.push_back({ true, handle, eventType, callback });

	return handle;
}

void ETJump::EventAggregator::unsubcribe(int eventHandle)
{
	for (auto & s : _serverCallbacks)
	{
		if (s.handle == eventHandle)
		{
			s.inUse = false;
			return;
		}
	}

	for (auto & c : _clientCallbacks)
	{
		if (c.handle == eventHandle)
		{
			c.inUse = false;
			return;
		}
	}
}

void ETJump::EventAggregator::clientEvent(ClientEventType eventType, const Payload *payload)
{
	for (const auto & c : _clientCallbacks)
	{
		if (c.type == eventType && c.inUse)
		{
			c.callback(payload);
		}
	}
}

void ETJump::EventAggregator::serverEvent(ServerEventType eventType, const Payload *payload)
{
	for (const auto & s : _serverCallbacks)
	{
		if (s.type == eventType && s.inUse)
		{
			s.callback(payload);
		}
	}
}
