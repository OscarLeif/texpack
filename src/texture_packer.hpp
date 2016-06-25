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
#include "util/point.hpp"
#include "util/vec2.hpp"
#include "util/rect.hpp"
#include "util/size.hpp"
#include "img/img.hpp"

#include <string>
#include <vector>

namespace binpack { struct rect_xywhf; }

struct sprite
{
	std::string name;
	std::string path;
	util::point offset;
	util::vec2 scale;
};

class texture_packer
{
public:
	struct cell
	{
		sprite sprite;
		bool flipped;
		int x;
		int y;
		unsigned w;
		unsigned h;
	};
	
private:
	img::img * _img;

	bool _alpha;
	bool _generated;
	std::vector<cell> _info;
	std::vector<sprite> _sprites;
	std::string _base_dir;

public:
	texture_packer(bool alpha, const std::string & base_dir);
	~texture_packer();

	bool add(const sprite & sprite);

	inline bool generated() const { return _generated; };
	inline const img::img * image() const { return _img; }
	inline std::vector<cell> info() const { return _info; }

	int pack();
	void save(std::string & fname, std::string & index_fname);

private:
	void pack_internal(const std::vector<util::rect> & rects, const std::vector<int> & order);
	void blit(const sprite & sprite, img::img * image, const binpack::rect_xywhf & blitrect);
};
