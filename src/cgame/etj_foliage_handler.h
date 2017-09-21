#pragma once
#include <vector>

namespace ETJump
{
	class FoliageHandler
	{
		const char *shaderName{ "__etjump-foliage-shader__" };
		std::vector<const char *> foliageShaders{
			"models/mapobjects/trees_sd/winterbranch01",
			"models/mapobjects/tree_temperate_sd/leaves_temperate1",
			"models/mapobjects/tree_temperate_sd/leaves_temperate2",
			"models/mapobjects/tree_temperate_sd/leaves_temperate3"
		};

		void turnOnFoliage();
		void turnOffFoliage();
	public:
		FoliageHandler();
		~FoliageHandler();
	};
}
