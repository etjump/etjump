#include "etj_cvar_master_drawer.h"
#include "../game/etj_numeric_utilities.h"

ETJump::CvarBasedMasterDrawer::CvarBasedMasterDrawer(const vmCvar_t& cvar) : selector(cvar)
{
	cvarUpdateHandler->subscribe(&selector, [&](const vmCvar_t* update)
	{
		updateCurrentIndex(update->integer);
	});
}

void ETJump::CvarBasedMasterDrawer::updateCurrentIndex(int index)
{
	currentIndex = Numeric::clamp(index, 1, renderables.size()) - 1;
}

void ETJump::CvarBasedMasterDrawer::beforeRender()
{
	if (shouldRender())
	{
		renderables[currentIndex]->beforeRender();
	}
}

void ETJump::CvarBasedMasterDrawer::render() const
{
	if (shouldRender())
	{
		renderables[currentIndex]->render();
	}
}

bool ETJump::CvarBasedMasterDrawer::shouldRender() const
{
	return etj_drawKeys.integer > 0 && renderables.size() > 0;
}

void ETJump::CvarBasedMasterDrawer::push(IRenderable* renderable)
{
	renderables.push_back(std::unique_ptr<IRenderable>(renderable));
	updateCurrentIndex(selector.integer);
}
