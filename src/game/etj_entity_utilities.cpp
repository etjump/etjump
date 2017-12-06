#include "etj_entity_utilities.h"
#include "etj_local.h"


bool ETJump::isPlayer(gentity_t* ent)
{
	auto cnum = ClientNum(ent);
	return cnum >= 0 && cnum < MAX_CLIENTS;
}
