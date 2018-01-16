#include <vector>
#include "etj_drawable.h"

static std::vector<Drawable *> _drawables;

void Drawable::addDrawable(Drawable *drawable)
{
	_drawables.push_back(drawable);
}

void ETJump_DrawDrawables()
{
	for (auto& drawable : _drawables)
	{
		drawable->draw();
	}
}

void ETJump_ClearDrawables()
{
	_drawables.clear();
}