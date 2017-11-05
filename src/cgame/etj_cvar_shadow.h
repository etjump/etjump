#pragma once
#include "cg_local.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <string>
#include <vector>

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