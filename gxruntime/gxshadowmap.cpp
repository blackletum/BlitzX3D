#include "std.h"
#include "gxshadowmap.h"
#include "gxgraphics.h"

std::set<gxShadowMap*>& gxShadowMap::registry() {
	static std::set<gxShadowMap*> _all;
	return _all;
}

gxShadowMap::gxShadowMap(gxGraphics* g, int res) :
	graphics(g), resolution(res),
	color_tex(nullptr), color_surf(nullptr), depth_surf(nullptr) {
	registry().insert(this);
	create();
}

gxShadowMap::~gxShadowMap() {
	registry().erase(this);
	destroy();
}

bool gxShadowMap::create() {
	destroy();

	IDirect3DDevice9Ex* dev = graphics->dir3dDev;
	if (!dev || resolution <= 0) return false;

	IDirect3DTexture9* tex = nullptr;
	HRESULT hr = dev->CreateTexture(resolution, resolution, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &tex, nullptr);
	if (FAILED(hr) || !tex) return false;

	IDirect3DSurface9* surf = nullptr;
	if (FAILED(tex->GetSurfaceLevel(0, &surf)) || !surf) {
		tex->Release();
		return false;
	}

	IDirect3DSurface9* zsurf = nullptr;
	hr = dev->CreateDepthStencilSurface(resolution, resolution, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, TRUE, &zsurf, nullptr);
	if (FAILED(hr) || !zsurf) {
		surf->Release();
		tex->Release();
		return false;
	}

	color_tex = tex;
	color_surf = surf;
	depth_surf = zsurf;
	return true;
}

void gxShadowMap::destroy() {
	if (depth_surf) { depth_surf->Release(); depth_surf = nullptr; }
	if (color_surf) { color_surf->Release(); color_surf = nullptr; }
	if (color_tex) { color_tex->Release(); color_tex = nullptr; }
}

void gxShadowMap::restore() {
	create();
}

void gxShadowMap::restoreAll() {
	for (gxShadowMap* s : registry()) s->restore();
}