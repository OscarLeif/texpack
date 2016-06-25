#include "fwriter.hpp"

#include <assert.h>

namespace core
{
	fwriter::fwriter()
		: _handle(nullptr)
	{ }
	
	fwriter::fwriter(const std::string & fname)
		: _handle(nullptr)
	{ open(fname, false, false); }

	fwriter::fwriter(const std::string & fname, bool binary)
		: _handle(nullptr)
	{ open(fname, binary, false); }

	fwriter::fwriter(const std::string & fname, bool binary, bool append)
		: _handle(nullptr)
	{ open(fname, binary, append); }

	fwriter::~fwriter()
	{ close(); }

	///////////////////////////////////////////////////////////////
	
	FILE * fwriter::handle()
	{ return _handle; }

	const FILE * fwriter::handle() const
	{ return _handle; }

	bool fwriter::open(const std::string & fname, bool binary, bool append)
	{
		//Create the open mode string
		char mode[4] = { "wt\0" };
		if (binary) mode[1] = 'b';
		if (append) mode[2] = '+';
		_binary = binary;

		close();
		_handle = fopen(fname.c_str(), mode);

		return ok();
	}
	
	bool fwriter::opened() const
	{ return _handle != nullptr; }
	
	void fwriter::close()
	{ if (opened()) fclose(_handle); }

	///////////////////////////////////////////////////////////////
	
	bool fwriter::ok() const
	{ return opened() && ferror(_handle) == 0; }

	///////////////////////////////////////////////////////////////
	
	long fwriter::pos() const
	{ return ftell(_handle); }
	
	void fwriter::seek(long offset, io::seekdir dir)
	{ fseek(_handle, offset, (int)dir); }

	///////////////////////////////////////////////////////////////

	void fwriter::write(const char * value)
	{
		write(value, sizeof(char)*strlen(value));
	}

	void fwriter::write(const std::string & value)
	{
		write((void*)value.c_str(), sizeof(std::string::value_type), value.size());
	}

	void fwriter::write_binstring(const std::string & value)
	{
		assert(value.length() < 256);
		write_uint8((uint8_t)value.length());
		write(value);
	}

	size_t fwriter::write(void * ptr, size_t elem_size, size_t elem_count)
	{
		return fwrite(ptr, elem_size, elem_count, _handle);
	}

	void fwriter::writeline()
	{ write<char>('\n'); }

	///////////////////////////////////////////////////////////////

	void fwriter::write_int8(int8_t value)
	{ write<int8_t>(value); }
	
	void fwriter::write_uint8(uint8_t value)
	{ write<uint8_t>(value); }
	
	void fwriter::write_int16(int16_t value)
	{ write<int16_t>(value); }
	
	void fwriter::write_uint16(uint16_t value)
	{ write<uint16_t>(value); }
	
	void fwriter::write_int32(int32_t value)
	{ write<int32_t>(value); }
	
	void fwriter::write_uint32(uint32_t value)
	{ write<uint32_t>(value); }
	
	void fwriter::write_int64(int64_t value)
	{ write<int64_t>(value); }
	
	void fwriter::write_uint64(uint64_t value)
	{ write<uint64_t>(value); }

	///////////////////////////////////////////////////////////////
	
	void fwriter::write_char(char value)
	{ write<char>(value); }
	
	void fwriter::write_uchar(unsigned char value)
	{ write<unsigned char>(value); }
	
	void fwriter::write_int(int value)
	{ write<int>(value); }
	
	void fwriter::write_uint(unsigned int value)
	{ write<unsigned int>(value); }
	
	void fwriter::write_short(short value)
	{ write<short>(value); }
	
	void fwriter::write_ushort(unsigned short value)
	{ write<unsigned short>(value); }
	
	void fwriter::write_long(long value)
	{ write<long>(value); }
	
	void fwriter::write_ulong(unsigned long value)
	{ write<unsigned long>(value); }
	
	void fwriter::write_float(float value)
	{ write<float>(value); }
	
	void fwriter::write_double(double value)
	{ write<double>(value); }
}
