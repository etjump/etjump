#include "etj_map_entity.h"
#include "g_local.h"

ETJump::MapEntity::MapEntity(gentity_t* entity) : Entity(entity)
{
	if (entity)
	{
		entity->use = [&](gentity_t *, gentity_t *other, gentity_t *activator)
		{
			use(other, activator);
		};
	}
}

ETJump::MapEntity::~MapEntity()
{
}
