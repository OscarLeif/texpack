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
#include <vector>
#include <string>
#include <experimental\filesystem>

namespace core
{
	namespace fs = ::std::experimental::filesystem;

	class freader;
	class fwriter;

	namespace io
	{
		//Seeking direction
		enum seekdir : int
		{
			current = SEEK_CUR,
			start = SEEK_SET,
			end = SEEK_END,
		};

		//Read a file
		freader read(const std::string & fname, bool binary = false);
		//Write to a file
		fwriter write(const std::string & fname, bool binary = false, bool append = false);

		//Read the contents for a file in a string
		bool read_content(const std::string & fname, std::string & output);
		//Read the contents for a file in a string
		bool read_content(const std::string & fname, char * output, long maxsize);
		//Read the lines of a text file into an array
		std::vector<std::string> read_lines(const std::string & fname, char delim = '\n');
		//Write the contents of a string to a file
		bool write_content(const std::string & fname, const std::string & content, bool append = false);
		//Write the contents of a string to a file
		bool write_content(const std::string & fname, const char * content, long size, bool append = false);
		//Append the contents of a file to another file
		bool append_content(const std::string & from, const std::string & to);
		//Append the contents of a file to another file
		bool append_content(const std::string & from, fwriter & to);

		//Last access time to the file
		time_t atime(const std::string & path);
		//Last modification time to the file
		time_t mtime(const std::string & path);
		//File creating time
		time_t ctime(const std::string & path);
	}
}