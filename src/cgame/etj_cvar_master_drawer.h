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
	class CvarBasedMasterDrawer : public IRenderable
	{
		std::vector<std::unique_ptr<IRenderable>> renderables;
		const vmCvar_t &selector;
		int currentIndex = 0;

		void updateCurrentIndex(int index);
		bool shouldRender() const;
	public:
		explicit CvarBasedMasterDrawer(const vmCvar_t& cvar);
		virtual ~CvarBasedMasterDrawer() {}
		void beforeRender() override;
		void render() const override;
		void push(IRenderable* renderable);
	};
}
