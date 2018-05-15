#pragma once
#include "cg_local.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "etj_irenderable.h"
#include <vector>
#include "etj_cvar_update_handler.h"

namespace ETJump
{
	class QuickFollowDrawer : public IRenderable
	{
		bool canSkipDraw() const;
	public:
		explicit QuickFollowDrawer();
		virtual ~QuickFollowDrawer() {}
		void beforeRender() override;
		void render() const override;
	};
}
