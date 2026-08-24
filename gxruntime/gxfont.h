#ifndef GXFONT_H
#define GXFONT_H

#include "std.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <vector>
#include <map>
#include <unordered_map>

#include "../gfx/gfxfont.h"

class gxCanvasD3D9;
class gxGraphicsD3D9;

// typedef IDirectDrawSurface7 ddSurf;

class gxFontD3D9 : public gxFont {
public:
	gxFontD3D9(FT_Library ftLibrary, gxGraphicsD3D9* gfx, const std::string& fn, int h, bool bold = false, bool italic = false, bool underlined = false);
	~gxFontD3D9();

	void render(gxCanvasD3D9* dest, unsigned color_argb, int x, int y, const std::string& t);

	int charWidth(int c) override;
	int charAdvance(int c) override;
	int stringWidth(const std::string& text) override;
	void setSmooth(bool enable) override { smooth = enable; }

	//ACCESSORS
	int getWidth()const override;							//width of widest char
	int getHeight()const override;							//height of font
	int getRenderOffset()const override;
	int getWidth(const std::string& text) override;	    //width of string
	bool isPrintable(int chr)const override;				//printable char?

	std::vector<gxCanvasD3D9*> atlases;

	gxCanvasD3D9* tempCanvas;

	enum {
		FONT_BOLD = 1,
		FONT_ITALIC = 2,
		FONT_UNDERLINE = 4 //TODO: remove? who actually wants this
	};

	bool bold;
	bool italic;
	bool underlined;
	bool smooth;
private:
	float getBaselinePosition()const;
	float getUnderlinePosition()const;
	float getUnderlineThickness()const;

	int maxWidth = 0;
	int glyphHeight = 0;
	int tCanvasHeight = 0;
	int glyphRenderBaseline = 0;
	int glyphRenderOffset = 0;

	struct GlyphData {
		int atlasIndex;
		int drawOffset[2];
		int horizontalAdvance;
		int srcRect[4];
	};

	const int atlasDims = 1024;
	void renderAtlas(int chr);
	int flags;

	int height;
	FT_Face freeTypeFace;
	std::unordered_map<int, GlyphData> glyphData;
	gxGraphicsD3D9* graphics;
	std::string filename;
};

#endif