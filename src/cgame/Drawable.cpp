#include "Drawable.h"
#include <vector>

static std::vector<Drawable *> _drawables;

void drawDrawables()
{
	for (auto& drawable : _drawables)
	{
		drawable->draw();
	}
}

void Drawable::addDrawable(Drawable *drawable)
{
	_drawables.push_back(drawable);
}