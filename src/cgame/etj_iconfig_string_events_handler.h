#pragma once
#include <string>
#include <functional>

namespace ETJump
{
	class IConfigStringEventsHandler
	{
	public:
		virtual ~IConfigStringEventsHandler()
		{
		}

		// subscribe to config string change event
		// returns an ID to unsubscribe
		virtual int subscribe(std::function<void(int configString, const std::string& value)> callback) = 0;
		// unsubscribe
		// non matching id is a no op
		virtual void unsubcribe(int id) = 0;
		// execute all callbacks
		virtual void configStringModified(int configString, std::string value) const = 0;
	};
}
