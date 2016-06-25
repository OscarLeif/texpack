#include "freader.hpp"

#include <varargs.h>

namespace core
{
	inline size_t min(size_t a, size_t b)
	{ return (a < b ? a : b); }

	freader::freader()
		: _handle(nullptr)
	{ }

	freader::freader(const std::string & fname)
		: _handle(nullptr)
	{ open(fname, false); }

	freader::freader(const std::string & fname, bool binary)
		: _handle(nullptr)
	{ open(fname, binary); }

	freader::~freader()
	{ close(); }

	///////////////////////////////////////////////////////////////

	bool freader::open(const std::string & fname, bool binary)
	{
		//Create the open mode string
		char mode[3] = { "rt" };
		if (binary) mode[1] = 'b';
		_binary = binary;
		
		close();
		_handle = fopen(fname.c_str(), mode);
		
		return ok();
	}

	bool freader::opened() const
	{ return (_handle != nullptr); }

	void freader::close()
	{
		if (!opened()) return;
		_binary = false;
		fclose(_handle);
	}

	///////////////////////////////////////////////////////////////

	bool freader::ok() const
	{ return (opened() && ferror(_handle) == 0); }

	bool freader::eof() const
	{ return (feof(_handle) != 0); }

	///////////////////////////////////////////////////////////////

	long freader::pos() const
	{ return (ftell(_handle)); }

	void freader::seek(long offset, io::seekdir dir)
	{ fseek(_handle, offset, (int)dir); }

	///////////////////////////////////////////////////////////////

	void freader::readrest(std::string & output)
	{
		long savedpos = pos();
		seek(0, io::seekdir::end);
		long lastpos = pos();
		seek(savedpos, io::seekdir::start);
		long restlen = (lastpos - savedpos);

		char * restdata = new char[restlen];
		read(restdata, restlen);
		output.insert(output.size(), restdata, restlen);
		delete[] restdata;
	}

	void freader::read(std::string & output, size_t count)
	{
		char * data = new char[count];
		read(data, count);
		output.append(data, data + count);
		delete[] data;
	}

	void freader::read_binstring(std::string & output)
	{ read(output, read_uint8()); }
	
	std::string freader::readline(char delim, size_t maxsize)
	{
		char c = '\0';
		std::string result;
		result.reserve(core::min(maxsize, size_t(100)));
		
		for (size_t i = 0; i < maxsize; i++)
		{
			read(c);
			if (eof()) break;
			result.push_back(c);
			if (c == delim) break;
		}

		return (result);
	}

	size_t freader::read(void * ptr, size_t elem_size, size_t elem_count)
	{
		return (fread(ptr, elem_size, elem_count, _handle));
	}

	///////////////////////////////////////////////////////////////

	int8_t freader::read_int8()
	{ return (read<int8_t>()); }

	uint8_t freader::read_uint8()
	{ return (read<uint8_t>()); }

	int16_t freader::read_int16()
	{ return (read<int16_t>()); }

	uint16_t freader::read_uint16()
	{ return (read<uint16_t>()); }

	int32_t freader::read_int32()
	{ return (read<int32_t>()); }

	uint32_t freader::read_uint32()
	{ return (read<uint32_t>()); }

	int64_t freader::read_int64()
	{ return (read<int64_t>()); }

	uint64_t freader::read_uint64()
	{ return (read<uint64_t>()); }

	///////////////////////////////////////////////////////////////

	char freader::read_char()
	{ return(read<char>()); }
	
	unsigned char freader::read_uchar()
	{ return(read<unsigned char>()); }
	
	int freader::read_int()
	{ return(read<int>()); }
	
	unsigned int freader::read_uint()
	{ return(read<unsigned int>()); }
	
	short freader::read_short()
	{ return(read<short>()); }
	
	unsigned short freader::read_ushort()
	{ return(read<unsigned short>()); }
	
	long freader::read_long()
	{ return(read<long>()); }
	
	unsigned long freader::read_ulong()
	{ return(read<unsigned long>()); }
	
	float freader::read_float()
	{ return(read<float>()); }
	
	double freader::read_double()
	{ return(read<double>()); }
}
