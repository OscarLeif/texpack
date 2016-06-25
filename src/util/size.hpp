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
	class point;

	//A size
	class size
	{
	public:
		using value_t = unsigned;

		//Width
		value_t width;
		//Height
		value_t height;

		//Construct a zero size {0, 0}
		size();
		//Construct a squared size {x, x}
		size(value_t square);
		//Construct a size {w, h}
		size(value_t width, value_t height);
		//Construct a size from a initializer list
		size(std::initializer_list<value_t> list);
		//Construct from another size
		size(const size & other) = default;

		size & multiply(float scalar);
		size & divide(float scalar);
		size & add(const size & other);
		size & subtract(const size & other);
		size & set(value_t width, value_t height);
		bool compare(const size & other) const;

		friend size operator * (size lhs, float rhs);
		friend size operator / (size lhs, float rhs);
		friend size operator + (size lhs, const size & rhs);
		friend size operator - (size lhs, const size & rhs);
		bool operator == (const size & rhs) const;
		bool operator != (const size & rhs) const;

		size & operator *= (float rhs);
		size & operator /= (float rhs);
		size & operator += (const size & rhs);
		size & operator -= (const size & rhs);
		size & operator = (const size & rhs) = default;

		point to_point() const;
	};
};