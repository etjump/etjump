#pragma once
#include <string>
#include <vector>
#include "cg_local.h"

namespace ETJump
{
	class CvarShadow
	{
		const vmCvar_t *_shadow;
		std::string _target;
	public:
		CvarShadow(const vmCvar_t *shadow, std::string target);
		~CvarShadow();
		void forceCvarSet(const vmCvar_t *cvar) const;
	};
}