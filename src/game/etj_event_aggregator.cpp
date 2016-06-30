#include "etj_event_aggregator.h"

ETJump::EventAggregator::EventAggregator(): _nextFreeHandle(0) {
}

ETJump::EventAggregator::~EventAggregator()
{
}

int ETJump::EventAggregator::subscribe(ServerEventType, std::function<void()>)
{
	auto handle = _nextFreeHandle++;
}

int ETJump::EventAggregator::subscribe(ClientEventType, std::function<void(int clientNum)>)
{
}

void ETJump::EventAggregator::unsubcribe(int eventHandle)
{
}
