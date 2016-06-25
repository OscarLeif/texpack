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
#include "point.hpp"
#include "size.hpp"

namespace util
{
	//An axis-aligned rectangle
	class rect
	{
	public:
		//Value type used for position and size of the rectangle
		using value_t = int;

		//X coorinate of the rectangle
		value_t x;
		//Y coordinate of the rectangle
		value_t y;
		//Width of the rectangle
		value_t w;
		//Height of the rectangle
		value_t h;

		//The zero rectangle ( at {0,0} with size {0,0} )
		static rect zero;
		//The unit rectangle ( at {0,0} with size {1,1} )
		static rect one;

		//Construct a zero rectangle
		rect();
		//Construct a rectangle at a position with given size
		rect(const point & position, const class util::size & sz);
		//Construct a rectangle at a position with given size
		rect(value_t x, value_t y, value_t size);
		//Construct a rectangle at a position with given size
		rect(value_t x, value_t y, value_t width, value_t height);

		//Get the position of the rectangle
		inline point position() const
		{ return point(x, y); }
		//Get the dimensions of the rectangle
		const class util::size dim() const
		{ return class util::size(w, h); }
		//Area of the rectangle
		inline value_t area() const
		{ return (w*h); }

		//Get top-left corner
		inline point top_left() const
		{ return point(x1(), y1()); }
		//Get top-right corner
		inline point top_right() const
		{ return point(x2(), y1()); }
		//Get bottom-left corner
		inline point bottom_left() const
		{ return point(x1(), y2()); }
		//Get bottom-right corner
		inline point bottom_right() const
		{ return point(x2(), y2()); }

		//Left side X coordnate
		value_t x1() const;
		//Top side Y coordinate
		value_t y1() const;
		//Right size X coordinate
		value_t x2() const;
		//Bottom side Y coordinate
		value_t y2() const;

		//Move the rectangle
		void translate(const point & offset);
		//Scale the rectnagle
		void scale(float scale);
		//Clip against another rectangle
		void clip_against(const rect & other);

		//Check another rectangle could fit into this one
		bool fits(const rect & other) const;
		//Check if this rectangle could fit into another
		bool fits_in(const rect & other) const;
		//Compare the sizes of two rectangles
		bool operator < (const rect & other) const;
		//Compare the sizes of two rectangles
		bool operator > (const rect & other) const;
		//Compare the sizes of two rectangles
		bool operator <= (const rect & other) const;
		//Compare the sizes of two rectangles
		bool operator >= (const rect & other) const;
		//Compare the sizes of two rectangles
		bool operator == (const rect & other) const;
		//Compare the sizes of two rectangles
		bool operator != (const rect & other) const;
	};
}
