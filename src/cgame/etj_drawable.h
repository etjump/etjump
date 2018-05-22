#ifndef DRAWABLE_H
#define DRAWABLE_H

class Drawable
{
public:
	Drawable()
	{
		addDrawable(this);
	}

	virtual ~Drawable()
	{
	};

	
	void removeFromDrawables();

	virtual void draw() = 0;

	static void addDrawable(Drawable *drawable);
};

#endif