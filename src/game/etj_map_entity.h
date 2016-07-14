#pragma once
#include "etj_entity.h"

namespace ETJump
{
	class MapEntity : public Entity
	{
	public:
		explicit MapEntity(gentity_t *entity);
		virtual ~MapEntity();

		// When activator tries to use this entity (e.g. presses F in front of a button or activates
		// a trigger)
		// @param other The trigger/button etc. that was activated
		// @param activator The activator itself
		virtual void use(gentity_t *other, gentity_t *activator) = 0;
		virtual void think(gentity_t *other, gentity_t *activator) = 0;
	};
}
