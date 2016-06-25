#include "img.hpp"
#include "png.hpp"
#include "jpeg.hpp"

namespace img
{
	img::img() : _data(nullptr), _w(0), _h(0) { }

	img::img(unsigned w, unsigned h)
		: _data(nullptr)
		, _w(w)
		, _h(h)
	{
		_data = new color[w*h];
	}

	img::~img()
	{
		if (_data != nullptr)
			delete[] _data;
	}

	color img::get(unsigned x, unsigned y) const
	{
		if (_data == nullptr) return color();
		if (x >= _w) return color();
		if (y >= _h) return color();

		return _data[y*_w + x];
	}

	void img::set(unsigned x, unsigned y, const color & c)
	{
		if (_data == nullptr) return;
		if (x >= _w) return;
		if (y >= _h) return;

		_data[y*_w + x] = c;
	}

	png * img::load_extended(const std::string & fname)
	{
		auto normal = loadimg(fname);
		if (normal == nullptr) return nullptr;
		png * result = new png(normal->w() + 2, normal->h() + 2);
		
		if (normal->w() > 0 && normal->h() > 0)
		{
			for (unsigned x = 0; x < normal->w(); ++x)
				for (unsigned y = 0; y < normal->h(); ++y)
					result->set(x + 1, y + 1, normal->get(x, y));

			for (unsigned x = 0; x < normal->w(); ++x)
			{
				result->set(x + 1, 0, normal->get(x, 1));
				result->set(x + 1, result->h() - 1, normal->get(x, normal->h() - 1));
			}

			for (unsigned y = 0; y < normal->h(); ++y)
			{
				result->set(0, y + 1, normal->get(1, y));
				result->set(result->w() - 1, y + 1, normal->get(normal->w() - 1, y));
			}
		}

		return result;
	}

	img * img::loadimg(const std::string & fname)
	{
		img * res = nullptr;

		//Try as PNG
		res = new png();
		try {
			res->load(fname);
			return (res);
		} catch (...)
		{ delete res; }

		//Try as JPEG
		res = new jpeg();
		try {
			res->load(fname);
			return (res);
		} catch (...)
		{ delete res; }

		//None of the types above
		return (nullptr);
	}

	img * img::loadimg(core::freader & reader)
	{
		img * res = nullptr;

		//Try as PNG
		res = new png();
		try {
			res->load(reader);
			return (res);
		} catch (...)
		{ delete res; }

		//Try as JPEG
		res = new jpeg();
		try {
			res->load(reader);
			return (res);
		} catch (...)
		{ delete res; }

		//None of the types above
		return (nullptr);
	}
}
