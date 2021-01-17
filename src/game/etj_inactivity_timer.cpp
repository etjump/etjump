#include "etj_inactivity_timer.h"

namespace ETJump
{
	void InactivityTimer::checkClientInactivity(gentity_t* ent)
	{
		if (ent->client->pers.cmd.buttons & BUTTON_ANY)
		{
			ent->client->realInactivityTime = level.time + 1000 * clientInactivityTimer;
			ent->client->sess.clientIsInactive = false;
			UpdateClientConfigString(*ent);
		}
		else if (level.time > ent->client->realInactivityTime)
		{
			ent->client->realInactivityTime = level.time + 1000 * clientInactivityTimer;
			ent->client->sess.clientIsInactive = true;
			UpdateClientConfigString(*ent);
		}
	}
}
