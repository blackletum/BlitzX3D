#ifndef GFXFONT_H
#define GFXFONT_H

#include <string>

class gxFont {
public:
	virtual ~gxFont() {}

	virtual int charWidth(int c) = 0;
	virtual int charAdvance(int c) = 0;
	virtual int stringWidth(const std::string& text) = 0;
	virtual void setSmooth(bool enable) = 0;

	//ACCESSORS
	virtual int getWidth()const = 0;							//width of widest char
	virtual int getHeight()const = 0;							//height of font
	virtual int getRenderOffset()const = 0;
	virtual int getWidth(const std::string& text) = 0;	//width of string
	virtual bool isPrintable(int chr)const = 0;				//printable char?

	enum {
		FONT_BOLD = 1,
		FONT_ITALIC = 2,
		FONT_UNDERLINE = 4
	};
};

#endif
