#include "point.hpp"

#include "size.hpp"
#include "rect.hpp"

namespace util
{
	const point point::zero(0, 0);

	const point point::one(1, 1);

	const point point::negative_one(-1, -1);

	////////////////////////////////////////////////

	point::point(value_t value)
		: x(value), y(value)
	{ }

	point::point(value_t x, value_t y)
		: x(x), y(y)
	{ }

	point::point(std::initializer_list<value_t> list)
	{
		if (list.size() == 1)
		{
			x = y = *list.begin();
			return;
		}

		if (list.size() == 2)
		{
			x = *list.begin();
			y = *list.end();
			return;
		}

		x = 0;
		y = 0;
	}

	////////////////////////////////////////////////

	point & point::set(value_t x, value_t y)
	{
		this->x = x;
		this->y = y;
		return *this;
	}

	////////////////////////////////////////////////

	point & point::multiply(float scalar)
	{
		x = int(x*scalar);
		y = int(y*scalar);
		return *this;
	}

	point & point::divide(float scalar)
	{
		x = int(x / scalar);
		y = int(y / scalar);
		return *this;
	}

	point & point::add(const point & other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	point & point::subtract(const point & other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}

	bool point::compare(const point & other) const
	{
		return (x == other.x) && (y == other.y);
	}

	////////////////////////////////////////////////

	point operator + (point lhs, const point & rhs)
	{ return lhs.add(rhs); }

	point operator - (point lhs, const point & rhs)
	{ return lhs.subtract(rhs); }

	bool point::operator == (const point & rhs) const
	{ return compare(rhs); }

	bool point::operator != (const point & rhs) const
	{ return !compare(rhs); }

	////////////////////////////////////////////////

	point & point::operator += (const point & rhs)
	{ return add(rhs); }

	point & point::operator -= (const point & rhs)
	{ return subtract(rhs); }

	////////////////////////////////////////////////

	bool point::is_inside(const rect & r)
	{
		return !(x < r.x1() || y < r.y1() || x > r.x2() || y > r.y2());
	}
}