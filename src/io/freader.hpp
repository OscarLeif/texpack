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

#include <istream>
#include <stdio.h>
#include <cstdint>
#include <string>

namespace core
{
	class freader
	{
		//Whether reading in binary mode
		bool _binary;
		//Internal file handle
		FILE * _handle;

	public:
		//Construct a file reader
		freader();
		//Construct a file reader
		freader(const std::string & fname);
		//Construct a file reader
		freader(const std::string & fname, bool binary);
		~freader();

		//Open another file (auto closes)
		bool open(const std::string & fname, bool binary);
		//Is a file opened
		bool opened() const;
		//Close the file
		void close();

		//Is reading in binary
		bool bin() const;
		//Is the file reader OK
		bool ok() const;
		//Is the EOF
		bool eof() const;
		
		//Get the current marker position
		long pos() const;
		//Move the marker to a different position
		void seek(long offset, io::seekdir dir = io::current);

		//Read an element from the file
		template<class T>
		void read(T & value);
		//Read an element from the file
		template<class T>
		T read();
		//Read N elements from the file
		template<class T>
		size_t read(T * value, size_t count);
		//Read a chunk into a string
		void read(std::string & output, size_t count);
		//Read a binary formatter string
		void read_binstring(std::string & output);
		//Read until EOF
		void readrest(std::string & output);
		//Read until delimeter reached (including delimeter)
		std::string readline(char delim = '\n', size_t maxsize = -1);
		//Read a chunk of memory into an array
		size_t read(void * ptr, size_t elem_size, size_t elem_count = 1);

		//Read an 8-bit int
		int8_t read_int8();
		//Read an 8-bit uint
		uint8_t read_uint8();
		//Read a 16-bit int
		int16_t read_int16();
		//Read a 16-bit uint
		uint16_t read_uint16();
		//Read a 32-bit int
		int32_t read_int32();
		//Read a 32-bit uint
		uint32_t read_uint32();
		//Read a 64-bit int
		int64_t read_int64();
		//Read a 64-bit uint
		uint64_t read_uint64();

		//Read a single char
		char read_char();
		//Read a single uchar
		unsigned char read_uchar();
		//Read a single int
		int read_int();
		//Read a single uint
		unsigned int read_uint();
		//Read a single short
		short read_short();
		//Read a single ushort
		unsigned short read_ushort();
		//Read a signle long
		long read_long();
		//Read a single ulong
		unsigned long read_ulong();
		//Read a single float
		float read_float();
		//Read a single double
		double read_double();
	};

	template<class T>
	void freader::read(T & value)
	{
		read((void*)&value, sizeof(T), 1);
	}

	template<class T>
	T freader::read()
	{
		T val;
		read(&val, sizeof(T), 1);
		return (val);
	}

	template<class T>
	size_t freader::read(T * value, size_t count)
	{
		return ( read((void*)value, sizeof(T), count) );
	}
}
