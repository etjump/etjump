#include "etj_cvar_update_handler.h"
#include <boost/algorithm/string/case_conv.hpp>

ETJump::CvarUpdateHandler::CvarUpdateHandler()
{
	callbacks.clear();
}

ETJump::CvarUpdateHandler::~CvarUpdateHandler() {}

bool ETJump::CvarUpdateHandler::check(const vmCvar_t *cvar)
{
	auto match = callbacks.find(cvar->handle);
	if (match != end(callbacks))
	{
		for (auto callback : match->second)
		{
			callback(cvar);
		}
		return true;
	}
	return false;
}

bool ETJump::CvarUpdateHandler::subscribe(const vmCvar_t *target, std::function<void(const vmCvar_t *cvar)> callback)
{
	callbacks[target->handle].push_back(callback);
	return true;
}


bool ETJump::CvarUpdateHandler::unsubscribe(const vmCvar_t *target)
{
	auto callback = callbacks.find(target->handle);
	if (callback != end(callbacks))
	{
		return false;
	}
	callbacks.erase(callback);
	return true;
}



