#pragma once
#include "etj_map_entity.h"

namespace ETJump
{
	class Checkpoint : public MapEntity
	{
	public:
		explicit Checkpoint(gentity_t *entity);
		~Checkpoint();

		void use(gentity_t* other, gentity_t* activator) override;
		void think(gentity_t* other, gentity_t* activator) override;
	};
}
