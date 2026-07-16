#ifndef GXSHADOWMAP_H
#define GXSHADOWMAP_H

#include <d3d9.h>
#include <set>

class gxGraphics;
class gxShadowMap {
public:
	gxShadowMap(gxGraphics* graphics, int resolution, bool cube);
	~gxShadowMap();

	bool isValid() const { return cube ? (cube_tex != nullptr) : (color_tex != nullptr); }
	bool isCube() const { return cube; }
	int  getResolution() const { return resolution; }

	IDirect3DTexture9* getTexture()const { return color_tex; }
	IDirect3DSurface9* getColorSurface()const { return color_surf; }
	IDirect3DCubeTexture9* getCubeTexture()const { return cube_tex; }
	IDirect3DSurface9* getCubeFaceSurface(int face)const { return cube_face_surf[face]; }
	IDirect3DSurface9* getDepthSurface()const { return depth_surf; }

	void restore();

	static void restoreAll();

private:
	gxGraphics* graphics;
	int resolution;
	bool cube;

	IDirect3DTexture9* color_tex;
	IDirect3DSurface9* color_surf;

	IDirect3DCubeTexture9* cube_tex;
	IDirect3DSurface9* cube_face_surf[6];

	IDirect3DSurface9* depth_surf;

	bool create();
	void destroy();

	static std::set<gxShadowMap*>& registry();
};

#endif