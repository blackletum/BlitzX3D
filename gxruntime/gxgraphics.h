#ifndef GXGRAPHICS_H
#define GXGRAPHICS_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <set>
#include <string>
#include <d3d9.h>

#include "ddutil.h"

#include "gxfont.h"
#include "gxcanvas.h"
#include "gxscene.h"
#include "gxmesh.h"
#include "gxmovie.h"
#include "../gfx/gfxgraphics.h"

class gxRuntime;
class gxEffectD3D9;

class gxGraphicsD3D9 : public gxGraphics {
public:
	IDirect3DDevice9Ex* dir3dDev;
	IDirect3DSurface9* frontBuffer;
	IDirect3DSurface9* backBuffer;
	IDirect3D9Ex* dir3d;

	D3DFORMAT           zbuffFmt;
	D3DPRESENT_PARAMETERS present_params;
	
	FT_Library ftLibrary;

	bool running_on_wine;

	gxGraphicsD3D9(gxRuntime* runtime, IDirect3DDevice9Ex* device, IDirect3DSurface9* front, IDirect3DSurface9* back, bool d3d);
	~gxGraphicsD3D9();

	bool restore();

	gxRuntime* runtime;
	//std::set<std::set<std::any>*> custom_set;

private:

	gxCanvasD3D9* front_canvas, * back_canvas;
	gxFontD3D9* def_font;
	bool gfx_lost;
	gxMeshD3D9* dummy_mesh;
	std::string lastEffectError;

	std::set<gxFontD3D9*> font_set;
	std::set<gxCanvasD3D9*> canvas_set;
	std::set<gxMeshD3D9*> mesh_set;
	std::set<gxSceneD3D9*> scene_set;
	std::set<gxMovieD3D9*> movie_set;
	std::set<std::string> font_res;
	std::set<gxEffectD3D9*> effect_set;

	// DDGAMMARAMP _gammaRamp;
	// IDirectDrawGammaControl* _gamma;

	/***** GX INTERFACE *****/
public:
	DeviceState getDeviceState() override;

	// i wonder what this is for
	gxEffectD3D9* createEffect(const std::string& filename) override;
	gxEffectD3D9* verifyEffect(gxEffect* e) override;
	void freeEffect(gxEffect* e) override;
	void clearEffects() override;
	const std::string& getLastEffectError() const override { return lastEffectError; }

	//MANIPULATORS
	void vwait() override;
	void flip(bool vwait) override;
	bool changeDisplayMode(int width, int height, bool fullscreen, bool borderless = false) override;

	//SPECIAL!
	void copy(gxCanvas* dest, int dx, int dy, int dw, int dh, gxCanvas* src, int sx, int sy, int sw, int sh) override;

	//NEW! Gamma control!
	void setGamma(int r, int g, int b, float dr, float dg, float db) override;
	void getGamma(int r, int g, int b, float* dr, float* dg, float* db) override;
	void updateGamma(bool calibrate) override;

	//ACCESSORS
	int getWidth()const override;
	int getHeight()const override;
	int getDepth()const override;
	int getScanLine()const override;
	int getAvailVidmem()const override;
	int getTotalVidmem()const override;

	gxCanvasD3D9* getFrontCanvas()const override;
	gxCanvasD3D9* getBackCanvas()const override;
	gxFontD3D9* getDefaultFont()const override;

	//OBJECTS
	gxCanvasD3D9* createCanvas(int width, int height, int flags) override;
	gxCanvasD3D9* loadCanvas(const std::string& file, int flags) override;
	gxCanvasD3D9* createCanvasFromImage(void* fib32, int w, int h, int flags) override;
	gxCanvasD3D9* verifyCanvas(gxCanvas* canvas) override;
	void freeCanvas(gxCanvas* canvas) override;
	void adoptCanvas(gxCanvas* c) override;

	bool imageHasAlpha(const std::string& file) override;
	const std::string& getLastImageError() const override;
	gxCanvas* loadTextureCanvas(const std::string& file, int flags, bool renderTarget, int* outW, int* outH) override;
	gxCanvas* createTextureCanvas(int w, int h, int flags, bool renderTarget) override;

	gxMovieD3D9* openMovie(const std::string& file, int flags) override;
	gxMovieD3D9* verifyMovie(gxMovieD3D9* movie);
	void closeMovie(gxMovie* movie) override;

	gxFontD3D9* loadFont(std::string font, int height, bool bold = false, bool italic = false, bool underlined = false) override;
	gxFontD3D9* verifyFont(gxFont* font) override;
	void freeFont(gxFont* font) override;

	gxSceneD3D9* createScene(int flags) override;
	gxSceneD3D9* verifyScene(gxScene* scene) override;
	void freeScene(gxScene* scene) override;

	gxMeshD3D9* createMesh(int max_verts, int max_tris, int flags) override;
	gxMeshD3D9* verifyMesh(gxMesh* mesh) override;
	void freeMesh(gxMesh* mesh) override;

	//GPU SKINNING
	bool skinningSupported() override;
	bool ensureSkinningShader() override;
	IDirect3DVertexShader9* getSkinningShader()const { return skin_vshader; }

	bool runningOnWine() const override { return running_on_wine; }

private:
	IDirect3DVertexShader9* skin_vshader;
	IDirect3DVertexDeclaration9* skin_decl;
	bool skin_shader_load_failed;
	int skin_caps_checked;   //-1 unknown, 0 unsupported, 1 supported
};

#endif