#ifndef GFXMOVIE_H
#define GFXMOVIE_H

#include <string>

class gxCanvas;

class gxMovie {
public:
	virtual ~gxMovie() {}

	virtual bool isValid() const = 0;

	virtual bool draw(gxCanvas* dest, int x, int y, int w, int h) = 0;
	virtual bool isPlaying() const = 0;
	virtual int getWidth()const = 0;
	virtual int getHeight()const = 0;
};

#endif
