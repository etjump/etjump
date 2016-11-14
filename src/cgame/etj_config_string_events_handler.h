#pragma once
#include <string>
#include <vector>
#include <functional>
#include "etj_iconfig_string_events_handler.h"

namespace ETJump
{
	class ConfigStringEventsHandler : public IConfigStringEventsHandler
	{
	public:
		struct Callback
		{
			std::function<void(int configString, const std::string& value)> handler;
			int id;
		};

		ConfigStringEventsHandler();
		~ConfigStringEventsHandler();

		// subscribe to config string change event
		// returns an ID to unsubscribe
		int subscribe(std::function<void(int configString, const std::string& value)> callback) override;
		// unsubscribe
		// non matching id is a no op
		void unsubcribe(int id) override;
		// execute all callbacks
		void configStringModified(int configString, std::string value) const override;
	private:
		int _nextCallbackId;
		std::vector<Callback> _callbacks;
	};
}

