#include "etj_cvar_shadow.h"
#include "etj_cvar_update_handler.h"

using namespace ETJump;

CvarShadow::CvarShadow(const vmCvar_t *shadow, std::string target) : _shadow(shadow), _target(target)
{
	forceCvarSet(shadow);
	;   cvarUpdateHandler->subscribe(_shadow, [&](const vmCvar_t *cvar)
	{
		forceCvarSet(cvar);
	});
};

CvarShadow::~CvarShadow()
{
}

void CvarShadow::forceCvarSet(const vmCvar_t *cvar) const
{
	trap_Cvar_Set(_target.c_str(), cvar->string);
}
