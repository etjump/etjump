#pragma once
#include "etj_map_entity.h"

namespace ETJump
{
	class StopTimer : public MapEntity
	{
	public:
		explicit StopTimer(gentity_t *entity);
		~StopTimer();

		void use(gentity_t* other, gentity_t* activator) override;
		void think(gentity_t* other, gentity_t* activator) override;
	};
}



