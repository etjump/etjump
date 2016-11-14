#include "etj_config_string_events_handler.h"
#include <algorithm>


ETJump::ConfigStringEventsHandler::ConfigStringEventsHandler():
	_nextCallbackId{0},
	_callbacks{}
{
}

ETJump::ConfigStringEventsHandler::~ConfigStringEventsHandler()
{
}

int ETJump::ConfigStringEventsHandler::subscribe(std::function<void(int configString, const std::string& value)> callback)
{
	_callbacks.push_back({callback, _nextCallbackId});
	return _nextCallbackId++;
}

void ETJump::ConfigStringEventsHandler::unsubcribe(int id)
{
	auto remove = std::find_if(begin(_callbacks), end(_callbacks), [&id](const Callback& cb)
	{
		return cb.id == id;
	});
	if (remove == end(_callbacks))
	{
		return;
	}
	_callbacks.erase(remove);
}

void ETJump::ConfigStringEventsHandler::configStringModified(int configString, std::string value) const
{
	for (const auto & cb : _callbacks)
	{
		cb.handler(configString, value);
	}
}
