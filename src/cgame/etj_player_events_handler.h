/*
	Any player related events, excluding spectated players or other players.
	Currently added:
	*	respawn ( revived: "1" or "0") 
	*	timerun:start ( runname, starttime, recordtime )
	*	timerun:stop ( runname, completiontime )
	*	timerun:record ( runname, completiontime )
	*	timerun:completion ( runname, compiletiontime )
	*	load
	add more as needed
*/
#pragma once

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <memory>

namespace ETJump
{
	class PlayerEventsHandler
	{
		std::map<std::string, std::vector<std::function<void(const std::vector<std::string>&)>>> _callbacks;
	public:
		PlayerEventsHandler();
		~PlayerEventsHandler();
		bool check(const std::string& event, const std::vector<std::string>& arguments);
		bool subscribe(const std::string& event, std::function<void(const std::vector<std::string>&)> callback);
		bool unsubscribe(const std::string& event);
	};
}



#pragma once
