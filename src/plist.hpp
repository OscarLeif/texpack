/*

Copyright (c) 2016 Botyto

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#pragma once

#if IMPLEMENT_KEYS
#  define defdeclkey(key, value) char key [] = value
#else
#  define defdeclkey(key, value) char key []
#endif

#define key(key, value) defdeclkey(key, value)

#define TAB "    "
#define TAB2 TAB TAB
#define TAB3 TAB2 TAB
#define TAB4 TAB3 TAB

#define PKEY(...) "<key>" __VA_ARGS__ "</key>"

namespace plist
{
	key(header,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n"
		TAB "<dict>\n"
		);

	key(frames_begin,
		TAB2 PKEY("frames") "\n"
		TAB2 "<dict>\n"
		);

	//file, offset X Y, size X Y, sourceSize X Y, rect X Y W H, rotated?
	key(frame,
		TAB3 PKEY("%s") "\n"
		TAB3 "<dict>\n"

		TAB4 PKEY("aliases") "\n"
		TAB4 "<array/>\n"

		TAB4 PKEY("anchor") "\n"
		TAB4 "<string>{0.0,0.0}</string>\n"

		TAB4 PKEY("spriteOffset") "\n"
		TAB4 "<string>{%d,%d}</string>\n"

		TAB4 PKEY("spriteSize") "\n"
		TAB4 "<string>{%d,%d}</string>\n"

		TAB4 PKEY("spriteSourceSize") "\n"
		TAB4 "<string>{%d,%d}</string>\n"

		TAB4 PKEY("textureRect") "\n"
		TAB4 "<string>{{%d,%d},{%d,%d}}</string>\n"

		TAB4 PKEY("textureRotated") "\n"
		TAB4 "<%s/>\n"

		TAB3 "</dict>\n"
		);

	key(frames_end,
		TAB2 "</dict>\n"
		);

	//realTextureFileName, size X Y, textureFileName
	key(metadata,
		TAB2 PKEY("metadata") "\n"
		TAB2 "<dict>\n"

		TAB3 PKEY("format") "\n"
		TAB3 "<integer>3</integer>\n"

		TAB3 PKEY("pixelFormat") "\n"
		TAB3 "<string>RGBA8888</string>\n"

		TAB3 PKEY("premultiplyAlpha") "\n"
		TAB3 "<false/>\n"

		TAB3 PKEY("realTextureFileName") "\n"
		TAB3 "<string>%s</string>\n"

		TAB3 PKEY("size") "\n"
		TAB3 "<string>{%d, %d}</string>\n"

		TAB3 PKEY("textureFileName") "\n"
		TAB3 "<string>%s</string>\n"

		TAB2 "</dict>\n"
		);

	key(footer,
		TAB "</dict>\n"
		"</plist>\n"
		);
}
