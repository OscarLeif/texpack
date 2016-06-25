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

#include "io/io.hpp"
#include <json/value.h>
#include <json/reader.h>

#include "texture_packer.hpp"

using namespace core;

std::vector<core::fs::path> spritesheet_list(const fs::path & directory)
{
	std::vector<fs::path> result;

	if (!fs::exists(directory) || !fs::is_directory(directory))
		return result;

	for (auto it : fs::directory_iterator(directory))
	{
		//Skip files (why are they there?)
		if (!fs::is_directory(it))
			continue;
		
		auto settings_path = it / it.path().filename();
		settings_path += ".json";
		if (!fs::exists(settings_path))
			continue;

		result.push_back(settings_path);
	}

	return result;
}

void process_atlas(const fs::path & settings_path, const fs::path & outdir)
{
	std::string settings_content;
	io::read_content(settings_path.string(), settings_content);

	json::reader reader;
	json::value settings;
	reader.parse(settings_content, settings, false);

	if (settings.type() != json::array_value)
	{
		printf("[TEx] Can't process '%s' - incorrect settings", settings_path.string().c_str());
		return;
	}

	printf("[TEX] Processing '%s' (%u sprites)\n", settings_path.stem().string().c_str(), settings.size());
	texture_packer packer(true, settings_path.parent_path().string());
	for (auto & cell : settings)
	{
		sprite spr;
		spr.name = cell["Name"].as_string();
		spr.path = cell["Path"].as_string();
		spr.offset.x = cell["Offset"]["X"].as_int();
		spr.offset.y = cell["Offset"]["Y"].as_int();
		spr.scale.x = cell["Scale"]["X"].as_float();
		spr.scale.y = cell["Scale"]["Y"].as_float();
		packer.add(spr);
	}
	
	fs::path plist = outdir;
	plist /= settings_path.stem();

	fs::path outimg = outdir;
	outimg /= settings_path.stem();

	packer.pack();
	packer.save(outimg.string(), plist.string());
}

void generate_sheet(const fs::path & input, const fs::path & outfile, int offx, int offy)
{
	json::value json;

	bool first = true;
	for (auto it : input)
	{
		if (!fs::is_regular_file(it))
			continue;

		json::value obj;

		obj["Path"] = it.string();
		obj["Name"] = it.filename().string();

		json::value offset;
		offset["X"] = offx;
		offset["Y"] = offy;
		obj["Offset"] = offset;

		json::value scale;
		scale["X"] = 1.0f;
		scale["Y"] = 1.0f;
		obj["Scale"] = scale;

		json.append(obj);
	}

	io::write_content(outfile.string(), json.to_styled_string());
}

void usage()
{
	printf("Usage:\n");
	printf("texpack -ps input/dir/ out/dir/\n");
	printf("texpack -gs input/dir/ out/file.json xoffset yoffset\n");
}

int pmain(int argn, char ** args)
{
	if (argn == 1)
	{
		usage();
		return 1;
	}

	if (strcmp(args[1], "-ps") == 0 && argn == 4)
	{
		fs::path graphics = args[2];
		fs::path outdir = args[3];

		fs::path spritesheets_dir = graphics / "spritesheets";
		auto sheets = spritesheet_list(spritesheets_dir);
		printf("[TEX] Processing %u spritesheet(s)\n", sheets.size());
		for (auto & sheet : sheets)
			process_atlas(sheet, outdir);

		return 0;
	}
	else if (strcmp(args[1], "-gs") == 0)
	{
		fs::path input = args[2];
		fs::path outfile = args[3];

		int xoffset = 0, yoffset = 0;
		if (argn == 6)
		{
			xoffset = atoi(args[4]);
			yoffset = atoi(args[5]);
		}

		generate_sheet(input, outfile, xoffset, yoffset);

		return 0;
	}
	else
	{
		usage();
		return 1;
	}

	return 0;
}

int main(int n, char ** v)
{
	int r = pmain(n, v);
#if _DEBUG
	system("pause");
#endif
	return r;
}