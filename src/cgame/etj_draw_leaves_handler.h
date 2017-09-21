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
			//"models/mapobjects/tree_desert_sd/palm_leaves",
			"models/mapobjects/tree_desert_sd/palm_leaves2",
		};

		void turnOnLeaves();
		void turnOffLeaves();
	public:
		DrawLeavesHandler();
		~DrawLeavesHandler();
	};
}
