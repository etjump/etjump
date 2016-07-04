#include "etj_entity_base.h"
#include "g_local.h"

ETJump::Entity::Entity(gentity_t* ent): _entity(ent ? ent : G_Spawn())
{
}

ETJump::Entity::~Entity()
{
	G_FreeEntity(_entity);
}

gentity_t* ETJump::Entity::entity() const
{
	return _entity;
}
