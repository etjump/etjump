#include <vector>
#include <algorithm>
#include "etj_drawable.h"

static std::vector<Drawable *> _drawables;

void Drawable::removeFromDrawables()
{
	auto it = std::find(_drawables.begin(), _drawables.end(), this);
	if ( it != _drawables.end())
	{
		_drawables.erase(it);
	}
}

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