#include "etj_player_events_handler.h"
#include <boost/algorithm/string/case_conv.hpp>

ETJump::PlayerEventsHandler::PlayerEventsHandler()
{
}

ETJump::PlayerEventsHandler::~PlayerEventsHandler()
{
	_callbacks.clear();
}

bool ETJump::PlayerEventsHandler::check(const std::string& event, const std::vector<std::string>& arguments)
{
	auto lowerEvent = boost::algorithm::to_lower_copy(event);
	auto match = _callbacks.find(lowerEvent);
	if (match != end(_callbacks))
	{
		for (auto listener : match->second) listener(arguments);
		return true;
	}
	return false;
}

bool ETJump::PlayerEventsHandler::subscribe(const std::string& event,
std::function<void(const std::vector<std::string>&)> callback)
{
	auto lowerEvent = boost::algorithm::to_lower_copy(event);
	_callbacks[lowerEvent].push_back(callback);
	return true;
}

bool ETJump::PlayerEventsHandler::unsubscribe(const std::string& event)
{
	auto lowerEvent = boost::algorithm::to_lower_copy(event);
	auto listeners = _callbacks.find(lowerEvent);
	if (listeners == end(_callbacks)) return false;
	_callbacks.erase(listeners);
	return true;
}
