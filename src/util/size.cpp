#include "size.hpp"

#include "point.hpp"

namespace util
{
	size::size()
		: width(0), height(0)
	{ }

	size::size(value_t square)
		: width(square), height(square)
	{ }

	size::size(value_t width, value_t height)
		: width(width), height(height)
	{ }

	size::size(std::initializer_list<value_t> list)
	{
		if (list.size() == 1)
		{
			width = height = *list.begin();
			return;
		}

		if (list.size() == 2)
		{
			width = *list.begin();
			height = *list.end();
			return;
		}

		width = 0;
		height = 0;
	}

	///////////////////////////////////////////////

	size & size::multiply(float scalar)
	{
		width = value_t(width*scalar);
		height = value_t(height*scalar);
		return *this;
	}

	size & size::divide(float scalar)
	{
		width = value_t(width / scalar);
		height = value_t(height / scalar);
		return *this;
	}

	size & size::add(const size & other)
	{
		width += other.width;
		height += other.height;
		return *this;
	}

	size & size::subtract(const size & other)
	{
		width -= other.width;
		height -= other.height;
		return *this;
	}

	size & size::set(value_t width, value_t height)
	{
		this->width = width;
		this->height = height;
		return *this;
	}

	bool size::compare(const size & other) const
	{
		return (width == other.width) && (height == other.height);
	}

	///////////////////////////////////////////////

	size operator * (size lhs, float rhs)
	{ return lhs.multiply(rhs); }

	size operator / (size lhs, float rhs)
	{ return lhs.divide(rhs); }

	size operator + (size lhs, const size & rhs)
	{ return lhs.add(rhs); }

	size operator - (size lhs, const size & rhs)
	{ return lhs.subtract(rhs); }

	bool size::operator == (const size & rhs) const
	{ return compare(rhs); }

	bool size::operator != (const size & rhs) const
	{ return !compare(rhs); }

	///////////////////////////////////////////////

	size & size::operator *= (float rhs)
	{ return multiply(rhs); }

	size & size::operator /= (float rhs)
	{ return divide(rhs); }

	size & size::operator += (const size & rhs)
	{ return add(rhs); }

	size & size::operator -= (const size & rhs)
	{ return subtract(rhs); }

	///////////////////////////////////////////////

	point size::to_point() const
	{ return point(width, height); }
}