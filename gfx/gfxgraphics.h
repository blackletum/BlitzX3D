#ifndef GFXGRAPHICS_H
#define GFXGRAPHICS_H

#include <string>

class gxCanvas;
class gxFont;
class gxScene;
class gxMesh;
class gxMovie;
class gxEffect;

class gxGraphics {
public:
	virtual ~gxGraphics() {}

	enum {
		GRAPHICS_WINDOWED = 1,	//windowed mode
		GRAPHICS_SCALED = 2,		//scaled window
		GRAPHICS_3D = 4,			//3d mode! Hurrah!
		GRAPHICS_AUTOSUSPEND = 8,	//suspend graphics when app suspended
		GRAPHICS_BORDERLESS = 16
	};

	enum DeviceState {
		DEVICE_OK,
		DEVICE_LOST,
		DEVICE_NEEDS_RESET
	};

	//effects
	virtual gxEffect* createEffect(const std::string& filename) = 0;
	virtual gxEffect* verifyEffect(gxEffect* e) = 0;
	virtual void freeEffect(gxEffect* e) = 0;
	virtual void clearEffects() = 0;
	virtual const std::string& getLastEffectError() const = 0;

	//MANIPULATORS
	virtual void vwait() = 0;
	virtual void flip(bool vwait) = 0;
	virtual bool changeDisplayMode(int width, int height, bool fullscreen, bool borderless = false) = 0;

	//SPECIAL!
	virtual void copy(gxCanvas* dest, int dx, int dy, int dw, int dh, gxCanvas* src, int sx, int sy, int sw, int sh) = 0;

	//NEW! Gamma control!
	virtual void setGamma(int r, int g, int b, float dr, float dg, float db) = 0;
	virtual void getGamma(int r, int g, int b, float* dr, float* dg, float* db) = 0;
	virtual void updateGamma(bool calibrate) = 0;

	//ACCESSORS
	virtual int getWidth()const = 0;
	virtual int getHeight()const = 0;
	virtual int getDepth()const = 0;
	virtual int getScanLine()const = 0;
	virtual int getAvailVidmem()const = 0;
	virtual int getTotalVidmem()const = 0;

	virtual gxCanvas* getFrontCanvas()const = 0;
	virtual gxCanvas* getBackCanvas()const = 0;
	virtual gxFont* getDefaultFont()const = 0;

	virtual DeviceState getDeviceState() = 0;

	//OBJECTS
	virtual gxCanvas* createCanvas(int width, int height, int flags) = 0;
	virtual gxCanvas* loadCanvas(const std::string& file, int flags) = 0;
	virtual gxCanvas* createCanvasFromImage(void* fib32, int w, int h, int flags) = 0;
	virtual gxCanvas* verifyCanvas(gxCanvas* canvas) = 0;
	virtual void freeCanvas(gxCanvas* canvas) = 0;
	virtual void adoptCanvas(gxCanvas* c) = 0;

	virtual bool imageHasAlpha(const std::string& file) = 0;
	virtual const std::string& getLastImageError() const = 0;
	virtual gxCanvas* loadTextureCanvas(const std::string& file, int flags, bool renderTarget, int* outW, int* outH) = 0;
	virtual gxCanvas* createTextureCanvas(int w, int h, int flags, bool renderTarget) = 0;

	virtual gxMovie* openMovie(const std::string& file, int flags) = 0;
	virtual void closeMovie(gxMovie* movie) = 0;

	virtual gxFont* loadFont(std::string font, int height, bool bold = false, bool italic = false, bool underlined = false) = 0;
	virtual gxFont* verifyFont(gxFont* font) = 0;
	virtual void freeFont(gxFont* font) = 0;

	virtual gxScene* createScene(int flags) = 0;
	virtual gxScene* verifyScene(gxScene* scene) = 0;
	virtual void freeScene(gxScene* scene) = 0;

	virtual gxMesh* createMesh(int max_verts, int max_tris, int flags) = 0;
	virtual gxMesh* verifyMesh(gxMesh* mesh) = 0;
	virtual void freeMesh(gxMesh* mesh) = 0;

	//GPU SKINNING
	virtual bool skinningSupported() = 0;
	virtual bool ensureSkinningShader() = 0;

	virtual bool runningOnWine() const = 0;
};

#endif
