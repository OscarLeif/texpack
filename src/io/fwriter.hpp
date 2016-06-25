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
#include "io_internal.hpp"

#include <stdio.h>
#include <cstdint>
#include <string>

namespace core
{
	class fwriter
	{
		//Internal file handle
		bool _binary;
		FILE * _handle;

	public:
		//Construct a file reader
		fwriter();
		//Construct a file reader
		fwriter(const std::string & fname);
		//Construct a file reader
		fwriter(const std::string & fname, bool binary);
		//Construct a file reader
		fwriter(const std::string & fname, bool binary, bool append);
		~fwriter();

		//Get native handle
		FILE * handle();
		//Get native handle
		const FILE * handle() const;
		//Open another file (auto closes)
		bool open(const std::string & fname, bool binary, bool append);
		//Is a file opened
		bool opened() const;
		//Close the file
		void close();

		//Is the file write OK
		bool ok() const;

		//Get the current marker position
		long pos() const;
		//Move the marker to a different position
		void seek(long offset, io::seekdir dir = io::current);

		//Write an element to the file
		template<class T>
		void write(const T & value);
		//Write a string to the file
		void write(const char * value);
		//Write a string to the file
		void write(const std::string & value);
		//Write binary formatted string (string preceeded by it's length as 1 byte)
		void write_binstring(const std::string & value);
		//Write N elements from the file
		template<class T>
		size_t write(T * value, size_t count);
		//Write a chunk of memory from an array
		size_t write(void * ptr, size_t elem_size, size_t elem_count = 1);
		//Write a new line character
		void writeline();

		//Write an 8-bit int
		void write_int8(int8_t value);
		//Write an 8-bit uint
		void write_uint8(uint8_t value);
		//Write a 16-bit int
		void write_int16(int16_t value);
		//Write a 16-bit uint
		void write_uint16(uint16_t value);
		//Write a 32-bit int
		void write_int32(int32_t value);
		//Write a 32-bit uint
		void write_uint32(uint32_t value);
		//Write a 64-bit int
		void write_int64(int64_t value);
		//Write a 64-bit uint
		void write_uint64(uint64_t value);

		//Write a single char
		void write_char(char value);
		//Write a single uchar
		void write_uchar(unsigned char value);
		//Write a single int
		void write_int(int value);
		//Write a single uint
		void write_uint(unsigned int value);
		//Write a single short
		void write_short(short value);
		//Write a single ushort
		void write_ushort(unsigned short value);
		//Write a signle long
		void write_long(long value);
		//Write a single ulong
		void write_ulong(unsigned long value);
		//Write a single float
		void write_float(float value);
		//Write a single double
		void write_double(double value);
	};

	template<class T>
	void fwriter::write(const T & value)
	{
		write((void*)&value, sizeof(T), 1);
	}
	
	template<class T>
	size_t fwriter::write(T * value, size_t count)
	{
		return write((void*)value, sizeof(T), count);
	}
}
