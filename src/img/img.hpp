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
#include "../util/rect.hpp"
#include "../io/fwriter.hpp"
#include "../io/freader.hpp"
#include "color.hpp"

#include <string>

namespace img
{
	class png;

	class img
	{
	protected:
		color * _data;
		unsigned _w;
		unsigned _h;

	public:
		using byte = unsigned char;

		//Construct an empty image
		img();
		//Construct an image
		img(unsigned w, unsigned h);
		//Destruct an image
		~img();

		//Image width
		inline unsigned w() const { return _w; }
		//Image height
		inline unsigned h() const { return _h; }
		
		//Image data beginning
		inline byte * data() { return (byte*)_data; }
		//Image data beginning
		inline const byte * data() const { return (const byte*)_data; }
		//Get color of specific pixel
		color get(unsigned x, unsigned y) const;
		//Set color to specific pixel
		void set(unsigned x, unsigned y, const color & c);
		//Fill a rect with specific color
		inline void fill(util::rect rect, const color & col) //TODO - see why this is here
		{
			//Fix rectangle sides to fit inside
			if (rect.x1() < 0) rect.x = 0;
			if (rect.y1() < 0) rect.y = 0;
			if (rect.x2() >= (int)_w) rect.w = _w - rect.x - 1;
			if (rect.y2() >= (int)_h) rect.h = _h - rect.y - 1;

			//Fill (:
			for (auto x = rect.x1(); x < rect.x2(); ++x)
				for (auto y = rect.y1(); y < rect.y2(); ++y)
					set(x, y, col);
		}

		//Save image
		virtual void save(const std::string & fname) = 0;
		//Load image
		virtual void load(const std::string & fname) = 0;
		//Load image
		virtual void load(core::freader & reader) = 0;

		//Load image with extensions
		static png * load_extended(const std::string & fname);

		//Load iamge
		static img * loadimg(const std::string & fname);
		//Load image
		static img * loadimg(core::freader & reader);
	};
}
