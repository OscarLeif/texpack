#include "vec2.hpp"
#include "point.hpp"

#include <cmath>
#include <limits>

namespace util
{
	const vec2 vec2::unit_x = vec2(1.0f, 0.0f);
	const vec2 vec2::unit_y = vec2(0.0f, 1.0f);
	const vec2 vec2::one = vec2(1.0f, 1.0f);
	const vec2 vec2::zero = vec2(0.0f, 0.0f);

	/////////////////////////////////////////////////////

	vec2::vec2(value_t value)
		: x(value), y(value)
	{ }

	vec2::vec2(value_t x, value_t y)
		: x(x), y(y)
	{ }

	/////////////////////////////////////////////////////

	vec2 & vec2::add(const vec2 & other)
	{
		x += other.x;
		y += other.y;

		return *this;
	}

	vec2 & vec2::subtract(const vec2 & other)
	{
		x -= other.x;
		y -= other.y;

		return *this;
	}

	vec2 & vec2::multiply(const vec2 & other)
	{
		x *= other.x;
		y *= other.y;

		return *this;
	}

	vec2 & vec2::divide(const vec2 & other)
	{
		x /= other.x;
		y /= other.y;

		return *this;
	}

	/////////////////////////////////////////////////////

	float vec2::length() const
	{
		return (std::sqrt(length_sqr()));
	}

	float vec2::length_sqr() const
	{
		return (x*x + y*y);
	}

	/////////////////////////////////////////////////////

	vec2 vec2::normalized() const
	{
		auto len = length();
		return vec2(x / len, y / len);
	}

	vec2 & vec2::normalize()
	{
		auto len = length();
		x /= len;
		y /= len;

		return (*this);
	}

	/////////////////////////////////////////////////////

	point vec2::to_point() const
	{
		return point((point::value_t)x, (point::value_t)y);
	}

	/////////////////////////////////////////////////////

	vec2 operator +(vec2 left, const vec2 & right)
	{
		return left.add(right);
	}

	vec2 operator -(vec2 left, const vec2 & right)
	{
		return left.subtract(right);
	}

	vec2 operator *(vec2 left, const vec2 & right)
	{
		return left.multiply(right);
	}

	vec2 operator /(vec2 left, const vec2 & right)
	{
		return left.divide(right);
	}

	/////////////////////////////////////////////////////

	vec2 & vec2::operator +=(const vec2 & other)
	{
		return add(other);
	}

	vec2 & vec2::operator -=(const vec2 & other)
	{
		return subtract(other);
	}

	vec2 & vec2::operator *=(const vec2 & other)
	{
		return multiply(other);
	}

	vec2 & vec2::operator /=(const vec2 & other)
	{
		return divide(other);
	}

	/////////////////////////////////////////////////////

	bool vec2::operator ==(const vec2 & other)
	{
		static const float eps = std::numeric_limits<float>::epsilon();
		return std::abs(x - other.x) < eps && std::abs(y - other.y) < eps;
	}

	bool vec2::operator !=(const vec2 & other)
	{
		return !(*this == other);
	}
}