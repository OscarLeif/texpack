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

#include "texture_packer.hpp"
#include "binpack.hpp"
#include "plist.hpp"

#include "img/img.hpp"
#include "img/png.hpp"
#include "img/jpeg.hpp"
#include "io/io.hpp"

#include <assert.h>

int max(int a, int b)
{
	return a < b ? a : b;
}

char itostr_out[2];
const char * itostr(int n)
{
	if (n < 0)
	{
		itostr_out[0] = '~';
		itostr_out[1] = '~';
	}
	if (n < 10)
	{
		itostr_out[0] = ' ';
		itostr_out[1] = '0' + n;
	}
	else
	{
		itostr_out[0] = '0' + (n/10);
		itostr_out[1] = '0' + (n%10);
	}

	return itostr_out;
}

////////////////////////////////////////////////////////////////////

texture_packer::texture_packer(bool alpha, const std::string & base_dir)
	: _img(nullptr)
	, _generated(false)
	, _alpha(alpha)
	, _base_dir(base_dir + "\\")
{
}

texture_packer::~texture_packer()
{
	if (_img != nullptr)
		delete _img;
}

////////////////////////////////////////////////////////////////////

bool texture_packer::add(const sprite & spr)
{
	if (_generated)
	{
		printf("[TEX] Spritesheet already generated!\n");
		return false;
	}

	if (!core::fs::exists(_base_dir + spr.path))
	{
		printf("[TEX] Can't find '%s'\n", spr.path.c_str());
		return false;
	}

	_sprites.push_back(spr);
	return true;
}

////////////////////////////////////////////////////////////////////

int texture_packer::pack()
{
	std::vector<int> order;
	std::vector<util::rect> rects;

	int i = 0;
	for (auto spr : _sprites)
	{
		try
		{
			auto img = img::img::load_extended(_base_dir + spr.path);
			rects.push_back(util::rect(0, 0, img->w(), img->h()));
			order.push_back(i);
			delete img;
		}
		catch (...)
		{
			printf("[TEX] '%s' has unsupported format\n", spr.path.c_str());
		}

		i++;
	}

	pack_internal(rects, order);
	return (_info.size());
}

void texture_packer::pack_internal(const std::vector<util::rect> & rects, const std::vector<int> & order)
{
	using namespace core;
	using binrect = binpack::rect_xywhf;

	//allocate binpack rectangles
	binrect ** rectptr = new binrect*[rects.size()];
	for (size_t i = 0; i < rects.size(); i++)
	{
		rectptr[i] = new binrect(0, 0, rects[i].w, rects[i].h);
		rectptr[i]->context = (void*)&_sprites[order[i]];
	}

	//attempt to package
	bool success;
	std::vector<binpack::bin> bins;
	//Final atlas sizes to try
	int final_size = 0;
	//All possible bin sizes are listed here
	//Nothing bigger can be used, because coordinates are packaged in only 2 bytes
	//Nothing bigger should be needed (:
	auto binsizes = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 }; 

	for (auto sz : binsizes)
	{
		bins.clear();
		success = binpack::bin::pack(rectptr, rects.size(), sz, bins);
		success = (bins.size() == 1);
		if (!success) continue;
		
		//stop if solution found
		final_size = sz;
		break;
	}

	//Celebrate ^^
	if (success)
	{
		_generated = true;

		//Create final image
		if (_alpha) _img = new img::png (final_size, final_size);
		else		_img = new img::jpeg(final_size, final_size);

		binpack::bin & res = bins[0];
		for (auto blitrect : res.rects)
		{
			auto spr = (sprite*)blitrect->context;
			try //try png
			{
				img::png * fpng = img::img::load_extended(_base_dir + spr->path);
				blit(*spr, fpng, *blitrect);
				delete fpng;
			}
			catch (...)
			{
				
				printf("[TEX] '%s' has usupported format\n", spr->path.c_str());
			}			
		}
	}

	//deallocate binpack rectangles
	for (size_t i = 0; i < rects.size(); i++)
		delete rectptr[i];
	delete[] rectptr;
}

void texture_packer::save(std::string & fname, std::string & index_fname)
{
	if (!_generated) pack();
	if (_img == nullptr) return;

	std::string img_ext = _alpha ? ".png" : ".jpeg";
	fname += img_ext;
	//core::console::info("[Atlas] Saving atlas to '%'\n", fname);
	_img->save(fname);

	std::string idx_ext = ".plist";
	index_fname += idx_ext;
	//core::console::info("[Atlas] Saving atlas index to '%'\n", index_fname);
	core::fwriter writer(index_fname, true);

	writer.write(plist::header);

	writer.write(plist::frames_begin);
	for (auto cell : _info)
	{
		sprite & spr = cell.sprite;
		util::size size(cell.w - 2, cell.h - 2);
		util::size scsize(size.width*spr.scale.x, size.height*spr.scale.y);
		util::point origin(cell.x + 1, cell.y + 1);
		
		//No idea how to fix the scaling

		fprintf(writer.handle(), plist::frame,
			//name
			spr.name.c_str(),
			//offset
			spr.offset.x, spr.offset.y,
			//size (-2 coz extensions for Cocos2D)
			size.width, size.height,
			//src size
			size.width, size.height,
			//src tex rect
			origin.x, origin.y, size.width, size.height,
			//flipped
			cell.flipped ? "true" : "false");
	}
	writer.write(plist::frames_end);

	core::fs::path outpath = fname;
	core::fs::path outfile = outpath.filename();
	std::string out = outfile.string();
	fprintf(writer.handle(), plist::metadata,
		//real tex fname
		out.c_str(),
		//size
		_img->w(), _img->h(),
		//tex fname
		out.c_str());

	writer.write(plist::footer);
}

////////////////////////////////////////////////////////////////////

void texture_packer::blit(const sprite & sprite, img::img * image, const binpack::rect_xywhf & blitrect)
{
	cell info;
	info.sprite = sprite;
	info.x = blitrect.x;
	info.y = blitrect.y;
	info.w = blitrect.w;
	info.h = blitrect.h;
	info.flipped = false;

	if (!blitrect.issquare())
	{
		//TODO - check why blitrect.flipped is incorrect
		if (blitrect.w == image->h())
		{
			assert(blitrect.h == image->w());
			info.flipped = true;
			//Fix flipped image bounds
			info.h--;
			info.y += info.h;
		}
	}

	_info.push_back(info);
	//printf("[Atlas] Blitting '%s'\n", fname.c_str());

	if (info.flipped)
	{
		for (unsigned yy = 0; yy < image->h(); yy++)
			for (unsigned xx = 0; xx < image->w(); xx++)
				_img->set(info.x + yy, info.y - xx, image->get(xx, yy));
	}
	else
	{
		for (unsigned yy = 0; yy < image->h(); yy++)
			for (unsigned xx = 0; xx < image->w(); xx++)
				_img->set(info.x + xx, info.y + yy, image->get(xx, yy));
	}
}
