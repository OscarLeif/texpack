#include "color.hpp"

namespace img
{
	color::color()
		: r(0xff), g(0xff), b(0xff), a(0xff)
	{ }

	color::color(unsigned code)
	{
		r = code >> 24;
		g = code >> 16;
		b = code >> 8;
		a = code >> 0;
	}

	color::color(byte red, byte green, byte blue, byte alpha)
		: r(red), g(green), b(blue), a(alpha)
	{ }

	//////////////////////////////////////////

	void color::set(unsigned code)
	{
		(*(unsigned*)this) = code;
	}

	void color::set(byte red, byte green, byte blue, byte alpha)
	{
		r = red;
		g = green;
		b = blue;
		a = alpha;
	}

	unsigned color::code() const
	{
		return *(unsigned*)this;
	}

	color::operator unsigned() const
	{
		return code();
	}

	void color::floats(float & r, float & g, float & b, float & a) const
	{
		r = this->r/255.0f;
		g = this->g/255.0f;
		b = this->b/255.0f;
		a = this->a/255.0f;
	}
}