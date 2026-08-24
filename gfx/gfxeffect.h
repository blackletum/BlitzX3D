#ifndef GFXEFFECT_H
#define GFXEFFECT_H

#include <string>

class gxCanvas;

class gxEffect {
public:
	virtual ~gxEffect() {}

	virtual bool setFloat(const std::string& name, float value) = 0;
	virtual bool setVector(const std::string& name, const float vec[4]) = 0;
	virtual bool setMatrix(const std::string& name, const float mat[16]) = 0;
	virtual bool setTexture(const std::string& name, gxCanvas* canvas) = 0;

	virtual bool begin(unsigned* passes) = 0;
	virtual bool beginPass(unsigned pass) = 0;
	virtual bool endPass() = 0;
	virtual bool end() = 0;
};

#endif
