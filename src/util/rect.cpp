#include "rect.hpp"

namespace util
{
	rect rect::zero(0, 0, 0, 0);
	rect rect::one(0, 0, 1, 1);

	//////////////////////////////////////////////////////

	rect::rect()
		: x(0), y(0), w(0), h(0)
	{ }

	rect::rect(const point & position, const util::size & size)
		: x(position.x), y(position.y), w(size.width), h(size.height)
	{ }

	rect::rect(value_t x, value_t y, value_t size)
		: x(x), y(y), w(size), h(size)
	{ }

	rect::rect(value_t x, value_t y, value_t width, value_t height)
		: x(x), y(y), w(width), h(height)
	{ }

	//////////////////////////////////////////////////////

	auto rect::x1() const -> value_t
	{ return (x); }

	auto rect::y1() const -> value_t
	{ return (y); }

	auto rect::x2() const -> value_t
	{ return (x + w); }

	auto rect::y2() const -> value_t
	{ return (y + h); }

	//////////////////////////////////////////////////////

	void rect::translate(const point & offset)
	{
		x += offset.x;
		y += offset.y;
	}

	void rect::scale(float scale)
	{
		w = (value_t)(w*scale);
		h = (value_t)(h*scale);
	}

	void rect::clip_against(const rect & other)
	{
		if (other.x2() < x2()) w = x2() - x1();
		if (other.y2() < y2()) h = other.y2() - y1();

		if (other.x1() > x1()) x = other.x1();
		if (other.y1() > y1()) y = other.y1();

		//correct possible invalid rect
		if (y1() > y2()) { y += h; h = 0; }
		if (x1() > x2()) { x += w; w = 0; }
	}

	//////////////////////////////////////////////////////
		
	bool rect::fits(const rect & other) const
	{
		return ((*this) >= other);
	}

	bool rect::fits_in(const rect & other) const
	{
		return ((*this) <= other);
	}

	bool rect::operator < (const rect & other) const
	{
		return (w < other.w && h < other.h);
	}

	bool rect::operator >(const rect & other) const
	{
		return (w > other.w && h > other.h);
	}

	bool rect::operator <= (const rect & other) const
	{
		return (w <= other.w && h <= other.h);
	}

	bool rect::operator >= (const rect & other) const
	{
		return (w >= other.w && h >= other.h);
	}

	bool rect::operator == (const rect & other) const
	{
		return (w == other.w && h == other.h);
	}

	bool rect::operator != (const rect & other) const
	{
		return (w != other.w || h != other.h);
	}
}
