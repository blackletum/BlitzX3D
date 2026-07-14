#ifndef GXSHADOWMAP_H
#define GXSHADOWMAP_H

#include <d3d9.h>
#include <set>

class gxGraphics;
class gxShadowMap {
public:
	gxShadowMap(gxGraphics* graphics, int resolution);
	~gxShadowMap();

	bool isValid() const { return color_tex != nullptr; }
	int  getResolution() const { return resolution; }

	IDirect3DTexture9* getTexture()const { return color_tex; }
	IDirect3DSurface9* getColorSurface()const { return color_surf; }
	IDirect3DSurface9* getDepthSurface()const { return depth_surf; }

	void restore();

	static void restoreAll();

private:
	gxGraphics* graphics;
	int resolution;

	IDirect3DTexture9* color_tex;
	IDirect3DSurface9* color_surf;
	IDirect3DSurface9* depth_surf;

	bool create();
	void destroy();

	static std::set<gxShadowMap*>& registry();
};

#endif