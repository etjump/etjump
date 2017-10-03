#pragma once
#include <vector>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace ETJump
{
	class DrawLeavesHandler
	{
		const char *shaderName{ "__etjump-leaves-shader__" };
		std::vector<const char *> leavesShaders{
			"models/mapobjects/trees_sd/winterbranch01",
			"models/mapobjects/tree_temperate_sd/leaves_temperate1",
			"models/mapobjects/tree_temperate_sd/leaves_temperate2",
			"models/mapobjects/tree_temperate_sd/leaves_temperate3",
			"models/mapobjects/tree_desert_sd/palm_leaf1",
			"models/mapobjects/tree_desert_sd/palm_leaves2",
			"models/mapobjects/plants_sd/bush_desert1",
			"models/mapobjects/plants_sd/bush_desert2",
			"models/mapobjects/plants_sd/bush_snow1",
			"models/mapobjects/plants_sd/catail1",
			"models/mapobjects/plants_sd/catail2",
			"models/mapobjects/plants_sd/deadbranch1",
			"models/mapobjects/plants_sd/deadbranch1_damp",
			"models/mapobjects/plants_sd/deadbranch2",
			"models/mapobjects/plants_sd/deadbranch3",
			"models/mapobjects/plants_sd/grassfoliage1",
			"models/mapobjects/plants_sd/grass_dry3",
			"models/mapobjects/plants_sd/grass_green1",
		};

		void turnOnLeaves();
		void turnOffLeaves();
	public:
		DrawLeavesHandler();
		~DrawLeavesHandler();
	};
}
