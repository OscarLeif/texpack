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

namespace util
{
	class point;

	//A 2D vector
	class vec2
	{
	public:
		using value_t = float;

		//The vector <1, 0>
		static const vec2 unit_x;
		//The vector <0, 1>
		static const vec2 unit_y;
		//The vector <0, 0>
		static const vec2 zero;
		//The vector <1, 1>
		static const vec2 one;

		//X component
		value_t x;
		//Y component
		value_t y;

		//Construct a zero 2D vector
		vec2() = default;
		//Construct a 2D vector where x = y = value
		vec2(value_t value);
		//Construct a 2D vector
		vec2(value_t x, value_t y);

		//Add a vector to this one
		vec2 & add(const vec2 & other);
		//Subtract a vector from this one
		vec2 & subtract(const vec2 & other);
		//Multiply both components of this vector with another one
		vec2 & multiply(const vec2 & other);
		//Divide both components of this vector with another one
		vec2 & divide(const vec2 & other);

		//Length of this vector
		float length() const;
		//Squared length of this vector
		float length_sqr() const;

		//A vector with the same allignment and a unit length
		vec2 normalized() const;
		//Normalize this vector
		vec2 & normalize();

		//Convert to a point
		point to_point() const;

		friend vec2 operator +(vec2 left, const vec2 & right);
		friend vec2 operator -(vec2 left, const vec2 & right);
		friend vec2 operator *(vec2 left, const vec2 & right);
		friend vec2 operator /(vec2 left, const vec2 & right);

		bool operator ==(const vec2 & other);
		bool operator !=(const vec2 & other);

		vec2 & operator +=(const vec2 & other);
		vec2 & operator -=(const vec2 & other);
		vec2 & operator *=(const vec2 & other);
		vec2 & operator /=(const vec2 & other);
	};
}