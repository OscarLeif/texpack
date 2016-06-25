//I'm sorry - I can't remember/find where I took this piece of code
//If anyone can recognize it (I modified it a tiny bit)
//then, please let me know at biserkrustev.botyto@gmail.com !!!

#pragma once
#include <vector>

/* of your interest:

* rect_xywhf - structure representing your rectangle object
* bin - structure representing your bin object
* bool rect2D(rect_xywhf* const * v, int n, int max_side, std::vector<bin>& bins) - actual packing function

v - pointer to array of pointers to your rectangle (" ** " design is for the sake of sorting speed and const means that the pointers will point to the same rectangles)
n - pointers' count
max_side - maximum bins' side - algorithm works with square bins (in the end it may trim them to rectangular form).
for the algorithm to finish faster, pass a reasonable value (unreasonable would be passing 1 000 000 000 for packing 4 50x50 rectangles).
bins - vector to which the function will push_back() created bins, each of them containing vector to pointers of rectangles from "v" belonging to that particular bin.
Every bin also keeps information about its width and height of course, none of the dimensions is bigger than max_side.

returns true on success, false if one of the rectangles' dimension was bigger than max_side

You want to pass your vector of rectangles representing your textures/glyph objects with GL_MAX_TEXTURE_SIZE as max_side,
then for each bin iterate through its rectangles, typecast each one to your own structure and then memcpy its pixel contents (rotated by 90 degrees if "flipped" rect_xywhf's member is true)
to the array representing your texture atlas to the place specified by the rectangle, then in the end upload it with glTexImage2D.

Algorithm doesn't create any new rectangles.
You just pass an array of pointers and rectangles' x/y/w/h are modified in place, with just vector of pointers for every new bin to let you know which ones belong to the particular bin.
Modifying w/h means that the dimensions can be actually swapped for the sake of fitting, the flag "flipped" will be set to true if such a thing occurs.

For description how to tune the algorithm and how it actually works see the .cpp file.

*/

namespace binpack
{
	struct rect_ltrb;
	struct rect_xywh;

	struct rect_wh
	{
		enum fits_result
		{
			no = 0,
			yes = 1,
			flipped = 2,
			perfect = 3,
			flipped_perfect = 4,
		};

		rect_wh(const rect_ltrb &);
		rect_wh(const rect_xywh &);
		rect_wh(int w = 0, int h = 0);

		int w;
		int h;

		int area() const;
		int perimeter() const;
		fits_result fits(const rect_wh & bigger) const;
	};

	// rectangle implementing left/top/right/bottom behaviour
	struct rect_ltrb
	{
		rect_ltrb();
		rect_ltrb(int left, int top, int right, int bottom);

		int l; int t;
		int r; int b;

		int w() const;
		int h() const;
		int area() const;
		int perimeter() const;

		void w(int w);
		void h(int h);
	};

	struct rect_xywh : public rect_wh
	{
		rect_xywh();
		rect_xywh(const rect_ltrb &);
		rect_xywh(int x, int y, int width, int height);
		operator rect_ltrb();

		int x;
		int y;

		int r() const;
		int b() const;

		void r(int r);
		void b(int b);
	};

	struct rect_xywhf : public rect_xywh
	{
		rect_xywhf(const rect_ltrb &);
		rect_xywhf(int x, int y, int width, int height);
		rect_xywhf();

		void flip();

		bool flipped;
		void * context;

		inline bool issquare() const { return w == h; };
	};

	struct bin
	{
		bin();

		rect_wh size;
		std::vector<rect_xywhf*> rects;

		static bool pack(rect_xywhf ** v, int n, int max_side, std::vector<bin> & bins);
	};
}
