#ifndef GXCANVAS_H
#define GXCANVAS_H

#include "ddutil.h"

#include "../gfx/gfxcanvas.h"

class gxFontD3D9;
class gxGraphicsD3D9;
class gxEffectD3D9;


class gxCanvasD3D9 : public gxCanvas {
public:
	gxCanvasD3D9(gxGraphicsD3D9* g, IDirect3DSurface9* surf, int flags);
	gxCanvasD3D9(gxGraphicsD3D9* g, IDirect3DTexture9* tex, int flags);
	gxCanvasD3D9(gxGraphicsD3D9* g, IDirect3DCubeTexture9* cube_tex, int flags);
	~gxCanvasD3D9();

	gxGraphicsD3D9* graphics;

	void backup() override;
	void restore() override;

	IDirect3DSurface9* getSurface()  const;
	IDirect3DBaseTexture9* getTexture() const;

	mutable int mod_cnt;
	mutable bool mipmapNeeded;

	mutable IDirect3DTexture9* blit_tex;
	mutable int blit_tex_mod_cnt;
	mutable unsigned blit_tex_mask;

	mutable int locked_pitch, locked_cnt, lock_mod_cnt, remip_cnt;
	mutable unsigned char* locked_surf;
	mutable bool lock_is_rt;

	PixelFormat format;

	RECT clip_rect;

	unsigned mask_surf, color_surf, color_argb, clsColor_surf;
	bool has_mask;

	void setModify(int n) override;
	int  getModify() const override;

	bool attachZBuffer();
	void releaseZBuffer();

	void restoreZBuffer();

	bool clip(RECT* d)          const;
	bool clip(RECT* d, RECT* s) const;
	void damage(const RECT& r)  const;

	void set2DEffect(gxEffect* effect) override;
	gxEffect* get2DEffect() const override;

	IDirect3DSurface9* surf;             // the "active" surf
	IDirect3DSurface9* z_surf;           // depth/stencil surf

private:
	int   flags, cube_mode;

	IDirect3DSurface9* plain_surf;   // non text offscreen surf
	IDirect3DTexture9* tex;
	IDirect3DCubeTexture9* cube_tex;

	IDirect3DSurface9* cube_surfs[6];

	mutable IDirect3DSurface9* t_surf;

	mutable int cm_pitch;
	mutable unsigned* cm_mask;

	gxEffectD3D9* effect2D;
	gxFontD3D9* font;
	RECT viewport;
	int origin_x, origin_y, handle_x, handle_y;

	void updateBitMask(const RECT& r) const;

	mutable int blit_batch_depth;
	mutable bool blit_batch_active;
	mutable void* blit_batch_saved;

	/***** GX INTERFACE *****/
public:
	void fillRect(const RECT& r, unsigned argb);

	//MANIPULATORS
	void setFont(gxFont* font) override;
	void setMask(unsigned argb) override;
	void setColor(unsigned argb) override;
	void setClsColor(unsigned argb) override;
	void setOrigin(int x, int y) override;
	void setHandle(int x, int y) override;
	void setViewport(int x, int y, int w, int h) override;

	void cls() override;
	void plot(int x, int y) override;
	void line(int x, int y, int x2, int y2) override;
	void rect(int x, int y, int w, int h, bool solid) override;
	void oval(int x, int y, int w, int h, bool solid) override;
	void text(int x, int y, const std::string& t) override;
	void blit(int x, int y, gxCanvas* src, int src_x, int src_y, int src_w, int src_h, bool solid) override;

	void blitstretch(int x, int y, int w, int h, gxCanvas* src, int src_x, int src_y, int src_w, int src_h, bool solid) override;//for CopyRectStretch

	void blitAlpha(int x, int y, gxCanvas* src, int src_x, int src_y, int src_w, int src_h, unsigned color_argb, bool filter = false) override;//for anti-aliased fonts

	bool collide(int x, int y, const gxCanvas* src, int src_x, int src_y, bool solid)const override;
	bool rect_collide(int x, int y, int rect_x, int rect_y, int rect_w, int rect_h, bool solid)const override;

	void beginBlitBatch() const override;
	void endBlitBatch() const override;

	bool lock()const override;
	bool isLocked()const override { return locked_cnt > 0; }
	unsigned char* getLockedSurf()const override { return locked_surf; }
	int getLockedPitch()const override { return locked_pitch; }
	void setPixel(int x, int y, unsigned argb) override;
	void setPixelFast(int x, int y, unsigned argb) override {
		format.setPixel(locked_surf + y * locked_pitch + x * format.getPitch(), argb);
		++mod_cnt;
	}
	void copyPixel(int x, int y, gxCanvas* src, int src_x, int src_y) override;
	void copyPixelFast(int x, int y, gxCanvas* src, int src_x, int src_y) override;
	unsigned getPixel(int x, int y)const override;
	unsigned getPixelFast(int x, int y)const override {
		return format.getPixel(locked_surf + y * locked_pitch + x * format.getPitch());
	};
	void unlock()const override;

	void setCubeMode(int mode) override;
	void setCubeFace(int face) override;

	int logical_w, logical_h;
	void setLogicalSize(int w, int h) override { logical_w = w; logical_h = h; }

	//ACCESSORS
	int getWidth()const override;
	int getHeight()const override;
	int getDepth()const override;
	int getFlags()const override { return flags; }
	int cubeMode()const override { return cube_mode; }
	void getOrigin(int* x, int* y)const override;
	void getHandle(int* x, int* y)const override;
	void getViewport(int* x, int* y, int* w, int* h)const override;
	unsigned getMask()const override;
	bool hasMask()const override { return has_mask; }
	void copyMaskFrom(const gxCanvas* src) override {
		const gxCanvasD3D9* s = static_cast<const gxCanvasD3D9*>(src);
		mask_surf = s->mask_surf; has_mask = s->has_mask;
	}
	unsigned getColor()const override;
	unsigned getClsColor()const override;
	bool hasAlphaMask()const override { return format.hasAlphaMask(); }

	IDirect3DBaseTexture9* getTexSurface() const;
	void setMipmapNeeded(bool needed) const { mipmapNeeded = needed; }
};

#endif