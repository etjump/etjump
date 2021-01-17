#pragma once

#include "g_local.h"

namespace ETJump
{
	class InactivityTimer
	{
	public:
		static void checkClientInactivity(gentity_t *ent);
	};
}
