#pragma once

#include "g_local.h"

namespace ETJump
{
	constexpr int clientInactivityTimer = 180; // in seconds

	class InactivityTimer
	{
	public:
		static void checkClientInactivity(gentity_t *ent);
	};
}
