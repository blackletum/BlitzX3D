#ifndef GXSCENE_H
#define GXSCENE_H

#include <map>
#include <d3d9.h>
#include <d3dx9.h>

#include "gxlight.h"
#include "gxeffect.h"
#include "../gfx/gfxscene.h"

class gxCanvasD3D9;

class gxMeshD3D9;
class gxLightD3D9;
class gxGraphicsD3D9;
class gxTexture;
class gxEffectD3D9;

class gxSceneD3D9 : public gxScene {
public:
	gxGraphicsD3D9* graphics;
	IDirect3DDevice9Ex* dir3dDev;

	gxSceneD3D9(gxGraphicsD3D9* graphics, gxCanvasD3D9* target);
	~gxSceneD3D9();


	/***** GX INTERFACE *****/
public:
	//state
	int  hwTexUnits() override;
	int  gfxDriverCaps3D() override;

	void setWBuffer(bool enable) override;
	void setHWMultiTex(bool enable) override;
	void setDither(bool enable) override;
	void setAntialias(bool enable) override;
	void setWireframe(bool enable) override;
	void setFlippedTris(bool enable) override;
	void setAmbient(const float rgb[]) override;
	void setAmbient2(const float rgb[]) override;
	void setFogColor(const float rgb[3]) override;
	void setFogRange(float nr, float fr) override;
	void setFogDensity(float den) override;
	void setFogMode(int mode) override;
	void setZMode(int mode) override;
	void setViewport(int x, int y, int w, int h) override;
	void setOrthoProj(float nr, float fr, float nr_w, float nr_h) override;
	void setPerspProj(float nr, float fr, float nr_w, float nr_h) override;
	void setViewMatrix(const Matrix* matrix) override;
	void setWorldMatrix(const Matrix* matrix) override;
	void setEyePosition(const float pos[3]) override;
	void setRenderState(const RenderState& state) override;
	void setEffect(gxEffect* effect) override;
	void setDepthTarget(gxCanvas* c) override;
	void setBumpNormalize(bool enable) override { bumpNormalize = enable; }

	void setTextureLodBias(float bias) override { textureLodBias = *((DWORD*)&bias); }
	void setTextureAnisotropic(int level) override { textureAnisotropic = level; }

	//rendering
	bool begin(const std::vector<gxLight*>& lights) override;
	void clear(const float rgb[3], float alpha, float z, bool clear_argb, bool clear_z) override;
	void render(gxMesh* mesh, int first_vert, int vert_cnt, int first_tri, int tri_cnt) override;
	void renderSkinned(gxMesh* mesh, int first_vert, int vert_cnt, int first_tri, int tri_cnt, const float* bone_data, int bone_cnt) override;
	void end() override;

	//lighting
	gxLight* createLight(int flags) override;
	void freeLight(gxLight* l) override;

	//info
	int getTrianglesDrawn()const override;
	gxEffect* getEffect() const override;

private:
	DWORD textureLodBias;
	int textureAnisotropic;

	gxCanvasD3D9* target;
	gxCanvasD3D9* depthTarget = nullptr;
	bool wbuffer, dither, antialias, wireframe, flipped;
	unsigned ambient, ambient2, fogcolor;
	int caps_level, fogmode, zmode, max_lights;
	float fogrange_nr, fogrange_fr, fog_density;
	D3DVIEWPORT9 viewport;
	bool ortho_proj;
	float frustum_nr, frustum_fr, frustum_w, frustum_h;
	D3DMATRIX projmatrix, viewmatrix, worldmatrix;
	D3DMATRIX inv_viewmatrix;
	D3DMATERIAL9 material;
	float shininess;
	int blend, fx;
	struct TexState {
		gxCanvasD3D9* canvas;
		int blend, flags;
		DWORD bumpEnvMat[2][2];
		DWORD bumpEnvScale;
		DWORD bumpEnvOffset;
		D3DMATRIX matrix;
		bool mat_valid;
	};
	TexState texstate[MAX_TEXTURES];
	int n_texs, tris_drawn;

	gxEffectD3D9* currentEffect;
	D3DXMATRIX currentWorld, currentView, currentProj;
	float eyePos[3];

	bool bumpNormalize = false;
	float bumpUniformScale = 1.0f;

	std::set<gxLightD3D9*> _allLights;
	std::vector<gxLightD3D9*> _curLights;

	int d3d_rs[160];
	int d3d_tss[8][32];
	int d3d_samp[8][16];
	IDirect3DBaseTexture9* d3d_tex[8];

	RenderState lastRenderState;
	bool lastRenderStateValid;

	uint64_t lastStateKey;

	void setRS(int n, int t);
	void setTSS(int n, int s, int t);
	void setSamp(int n, int s, int t);
	void setTex(int n, IDirect3DBaseTexture9* t);

	void setLights();
	void setZMode();
	void setAmbient();
	void setFogMode();
	void setTriCull();
	void setTexState(int index, const TexState& state, bool set_blend);
	void setEffectInternal(gxEffectD3D9* e);
	void setSkinShaderConstants();
};

#endif
