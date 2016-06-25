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

namespace img
{
	//A RGBA color
	class color
	{
	public:
		using byte = unsigned char;

		//Red channel
		byte r;
		//Green channel
		byte g;
		//Blue channel
		byte b;
		//Alpha channel
		byte a;

		//Construct as black opaque color
		color();
		//Construct from a color code
		color(unsigned code);
		//Construct from both channels
		color(byte red, byte green, byte blue, byte alpha = 255);
		//Copy construct a color
		color(const color & other) = default;
		color & operator = (const color & other) = default;

		//Set using a color code
		void set(unsigned code);
		//Set both channels individually
		void set(byte red, byte green, byte blue, byte alpha = 255);

		//Get the color code
		unsigned code() const;
		//Get the color code
		operator unsigned() const;
		//Get the color as floats
		void floats(float & r, float & g, float & b, float & a) const;
	};
}