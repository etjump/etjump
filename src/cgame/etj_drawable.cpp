#include "Drawable.h"
#include <vector>

static std::vector<Drawable *> _drawables;

void Drawable::addDrawable(Drawable *drawable)
{
	_drawables.push_back(drawable);
}

extern "C" void ETJump_DrawDrawables()
{
	for (auto& drawable : _drawables)
	{
		drawable->draw();
	}
}

extern "C" void ETJump_ClearDrawables()
{
	_drawables.clear();
}