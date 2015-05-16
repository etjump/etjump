#ifndef DRAWABLE_H
#define DRAWABLE_H

class Drawable
{
public:
    Drawable() {
        addDrawable(this);
    }

    virtual ~Drawable() {};

    virtual void draw() = 0;

    static void addDrawable(Drawable *drawable);
};

void drawDrawables();

#endif