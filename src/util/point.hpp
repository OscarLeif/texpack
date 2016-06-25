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
#include <initializer_list>

namespace util
{
	class size;
	class rect;

	//A 2D point class
	class point
	{
	public:
		//The type of both components
		using value_t = int;

		//The point <0, 0>
		static const point zero;
		//The point <1, 1>
		static const point one;
		//The point <-1, -1>
		static const point negative_one;

		//X component
		value_t x;
		//Y component
		value_t y;

		point() = default;
		point(value_t value);
		point(value_t x, value_t y);
		point(std::initializer_list<value_t> list);
		point(const point & other) = default;

		point & set(value_t x, value_t y);
		point & multiply(float scalar);
		point & divide(float scalar);
		point & add(const point & other);
		point & subtract(const point & other);
		bool compare(const point & other) const;

		friend point operator + (point lhs, const point & rhs);
		friend point operator - (point lhs, const point & rhs);
		bool operator == (const point & rhs) const;
		bool operator != (const point & rhs) const;

		point & operator += (const point & rhs);
		point & operator -= (const point & rhs);
		point & operator = (const point & rhs) = default;

		bool is_inside(const rect & rectangle);
		size to_size() const;
	};
}