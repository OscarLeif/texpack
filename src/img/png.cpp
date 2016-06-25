#include "png.hpp"
#include <stdio.h>
#include <pngstruct.h>

namespace
{
	void my_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		core::freader & reader = *(core::freader*)(png_ptr->io_ptr);
		reader.read(data, length);
	}

	void my_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		core::fwriter & writer = *(core::fwriter*)(png_ptr->io_ptr);
		writer.write(data, length);
	}

	void my_flush_data(png_structp png_ptr)
	{
		//whut?
	}

	void my_error_handler(png_structp png_ptr, png_const_charp error_msg)
	{
		printf("[PNG] %s\n", error_msg);
	}

	void my_warning_handler(png_structp png_ptr, png_const_charp warning_msg)
	{
		printf("[PNG] %s\n", warning_msg);
	}
}

namespace img
{
	using fptr_t = color(*)(color::byte*);

	color getrgb(color::byte * ptr)
	{
		return color(ptr[0], ptr[1], ptr[2], 255);
	}

	color getrgba(color::byte * ptr)
	{
		return color(ptr[0], ptr[1], ptr[2], ptr[3]);
	}

	///////////////////////////////////////////////////////////////////////////

	png::png(unsigned w, unsigned h)
		: img(w, h)
		, _coltype(PNG_COLOR_TYPE_RGBA)
		, _depth(8)
	{ }

	png::png(const std::string & fname)
		: img()
	{
		load(fname);
	}

	png::png() : img() { }

	///////////////////////////////////////////////////////////////////////////

	void png::save(const std::string & fname)
	{
		//Check if data isn't present
		if (_data == nullptr)
			throw std::exception("data isn't present");

		//create file
		auto fp = core::io::write(fname, true, false);
		if (!fp.opened() || !fp.ok()) throw std::exception("cant open for writing");

		//initialize stuff
		png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png) throw std::exception("cant create png struct");

		png_infop info = png_create_info_struct(png);
		if (!info) throw std::exception("cant create png info");

		//why first?
		if (setjmp(png_jmpbuf(png)))
			throw std::exception("cant init io");
		png_set_error_fn(png, nullptr, &my_error_handler, &my_warning_handler);
		png_set_write_fn(png, &fp, &my_write_data, &my_flush_data);

		//write header
		if (setjmp(png_jmpbuf(png)))
			throw std::exception("error writing header");

		//set header
		png_set_IHDR(png, info, _w, _h,
			_depth, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
		//write header
		png_write_info(png, info);

		//write bytes
		if (setjmp(png_jmpbuf(png)))
			throw std::exception("error writing data");

		byte ** rows = new byte*[_h];
		for (unsigned y = 0; y < _h; ++y)
		{
			rows[y] = new byte[_w * 4];
			for (unsigned x = 0; x < _w; ++x)
			{
				color & c = _data[y*_w + x];
				rows[y][x * 4 + 0] = c.r;
				rows[y][x * 4 + 1] = c.g;
				rows[y][x * 4 + 2] = c.b;
				rows[y][x * 4 + 3] = c.a;
			}
		}

		png_write_image(png, rows);

		//end write
		if (setjmp(png_jmpbuf(png)))
			throw std::exception("error writing end");
		png_write_end(png, NULL);

		//cleanup heap allocation
		for (unsigned y = 0; y < _h; ++y)
			delete[] rows[y];
		delete[] rows;

		fp.close();
	}

	void png::load(const std::string & fname)
	{
		//cleanup data
		if (_data != nullptr)
			delete[] _data;

		//Open file
		auto fp = core::io::read(fname, true);
		if (!fp.opened() || !fp.ok())
			throw std::exception("cant open file");

		//Read using file reader object
		load(fp);

		//close
		fp.close();
	}

	void png::load(core::freader & reader)
	{
		//Read header
		char header[8];
		reader.read(header, 8);
		if (png_sig_cmp((png_const_bytep)header, 0, 8))
			throw std::exception("not png");

		//Create PNG struct
		png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png) throw std::exception("Cant create PNG struct");

		png_infop info = png_create_info_struct(png);
		if (!info) throw std::exception("Cant create info struct");

		//jump buffer?
		if (setjmp(png_jmpbuf(png)))
			throw std::exception("cant init io ?");

		//read basic data
		//png_init_io(png, fp);
		png_set_error_fn(png, nullptr, &my_error_handler, &my_warning_handler);
		png_set_read_fn(png, &reader, &my_read_data);
		png_set_sig_bytes(png, 8);
		png_read_info(png, info);
		_w = png_get_image_width(png, info);
		_h = png_get_image_height(png, info);

		_coltype = png_get_color_type(png, info);
		_depth = png_get_bit_depth(png, info);

		_passes = png_set_interlace_handling(png);
		png_read_update_info(png, info);

		if (setjmp(png_jmpbuf(png)))
			throw std::exception("error reading image");

		byte ** rows = new byte*[_h];
		int rowsize = png_get_rowbytes(png, info);
		for (unsigned y = 0; y < _h; ++y)
			rows[y] = new byte[rowsize];

		//Read data
		png_read_image(png, rows);

		//get png type
		byte pngtype = png_get_color_type(png, info);
		//if (pngtype != PNG_COLOR_TYPE_RGB || pngtype != PNG_COLOR_TYPE_RGBA)
		//throw core::exception("not rgb or rgba");

		fptr_t fptr = (pngtype == PNG_COLOR_TYPE_RGB) ? &getrgb : &getrgba;
		unsigned step = (pngtype == PNG_COLOR_TYPE_RGB) ? 3 : 4;

		_data = new color[_h*_w];
		for (unsigned y = 0; y < _h; ++y)
		{
			png_byte * row = rows[y];
			for (unsigned x = 0; x < _w; ++x)
			{
				byte * ptr = &row[x * step];
				_data[y*_w + x] = (*fptr)(ptr);
			}

			//delete[] rows[y]; //TODO - check if this can go here and implement for jpeg
		}

		//clean up
		for (unsigned y = 0; y < _h; ++y)
			delete[] rows[y];
		delete[] rows;
	}
}
