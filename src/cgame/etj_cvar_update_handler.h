#pragma once
#include "cg_local.h"
#include <string>
#include <functional>
#include <map>
#include <vector>

namespace ETJump
{
	class CvarUpdateHandler
	{
		std::map<int, std::vector<std::function<void(const vmCvar_t*)>>> callbacks;
	public:
		CvarUpdateHandler();
		~CvarUpdateHandler();
		bool check(const vmCvar_t *cvar);
		bool subscribe(const vmCvar_t *target, std::function<void(const vmCvar_t *cvar)> callback);
		bool unsubscribe(const vmCvar_t *target);
	};
}