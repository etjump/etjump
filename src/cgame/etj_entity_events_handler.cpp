#include "etj_entity_events_handler.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/format.hpp>

ETJump::EntityEventsHandler::EntityEventsHandler()
{
	_callbacks.clear();
}

ETJump::EntityEventsHandler::~EntityEventsHandler()
{
}

bool ETJump::EntityEventsHandler::check(const std::string &eventName, centity_t *cent)
{
	auto lowercaseCommand = boost::algorithm::to_lower_copy(eventName);
	auto match = _callbacks.find(lowercaseCommand);
	if (match != end(_callbacks))
	{
		for (auto callback : match->second)
		{
			callback(cent);
		}
		return true;
	}
	return false;
}

bool ETJump::EntityEventsHandler::check(int event, centity_t *cent)
{
	auto eventName = (boost::format{ "__event__%d" } % event).str();
	return check(eventName, cent);
}


bool ETJump::EntityEventsHandler::subscribe(const std::string& eventName, std::function<void(centity_t* cent)> callback)
{
	auto lowercaseCommand = boost::algorithm::to_lower_copy(eventName);
	_callbacks[lowercaseCommand].push_back(callback);
	return true;
}

bool ETJump::EntityEventsHandler::subscribe(int event, std::function<void(centity_t *cent)> callback)
{
	auto eventName = (boost::format { "__event__%d" } % event).str();
	return subscribe(eventName, callback);
}


bool ETJump::EntityEventsHandler::unsubcribe(const std::string &eventName)
{
	auto lowercaseCommand = boost::algorithm::to_lower_copy(eventName);
	auto callback = _callbacks.find(lowercaseCommand);
	if (callback != end(_callbacks))
	{
		return false;
	}

	_callbacks.erase(callback);
	return true;
}

bool ETJump::EntityEventsHandler::unsubcribe(int event)
{
	auto eventName = (boost::format { "__event__%d" } % event).str();
	return unsubcribe(eventName);
}

