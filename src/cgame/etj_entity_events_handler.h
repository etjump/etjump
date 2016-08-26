#pragma once
#include "cg_local.h"
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <memory>

namespace ETJump
{
	class EntityEventsHandler
	{
	public:
		explicit EntityEventsHandler();
		~EntityEventsHandler();

		// returns true if a match was found and function was called
		bool check(const std::string &eventName, centity_t *cent);
		bool check(int event, centity_t *cent);

		// registers a event handler that will be called if the event was received from the server
		bool subscribe(const std::string &eventName, std::function<void(centity_t *cent)> callback);
		bool subscribe(int event, std::function<void(centity_t *cent)> callback);

		// unsubscribes the event handler
		// returns false if it does not exist
		bool unsubcribe(const std::string &eventName);
		bool unsubcribe(int event);

	private:
		std::map<std::string,  std::vector<std::function<void(centity_t *cent)>>> _callbacks;
	};
}
