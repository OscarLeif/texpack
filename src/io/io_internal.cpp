#include "io_internal.hpp"
#include "freader.hpp"
#include "fwriter.hpp"
#include <sys/stat.h>

namespace
{
	void rstrip(std::string & str)
	{
		std::string skip = "\t\r\n ";
		while (skip.find(str.back()) != std::string::npos)
			str.pop_back();
	}
}

namespace core
{
	namespace io
	{
		freader read(const std::string & fname, bool binary)
		{
			return (freader(fname, binary));
		}

		fwriter write(const std::string & fname, bool binary, bool append)
		{
			return (fwriter(fname, binary, append));
		}

		bool read_content(const std::string & fname, std::string & output)
		{
			freader reader(fname, true);
			if (!reader.ok())
				return 1;

			reader.seek(0, seekdir::end);
			long size = reader.pos();
			output.reserve(output.size() + size);
			reader.seek(0, seekdir::start);

			char * buff = new char[size];
			reader.read(buff, size);
			output.append(buff, buff + size);
			delete[] buff;

			return 0;
		}

		bool read_content(const std::string & fname, char * output, long maxsize)
		{
			freader reader(fname, true);
			if (!reader.ok())
				return 1;

			reader.seek(0, seekdir::end);
			long size = std::min(reader.pos(), maxsize);
			reader.seek(0, seekdir::start);

			reader.read(output, size);
			return 0;
		}

		std::vector<std::string> read_lines(const std::string & fname, char delim)
		{
			std::vector<std::string> result;
			freader reader(fname, true);
			if (!reader.ok())
				return result;

			std::string line;
			while (!reader.eof())
			{
				line = reader.readline(delim);
				if (line.empty()) continue;
				rstrip(line);
				if (line.empty()) continue;
				result.push_back(line);
			}

			return result;
		}

		bool write_content(const std::string & fname, const std::string & content, bool append)
		{
			fwriter writer(fname, true, append);
			if (!writer.ok())
				return 1;

			writer.write(content);
			return 0;
		}

		bool write_content(const std::string & fname, const char * content, long size, bool append)
		{
			fwriter writer(fname, true, append);
			if (!writer.ok())
				return 1;

			writer.write(content, size);
			return 0;
		}

		bool append_content(const std::string & from, const std::string & to)
		{
			fwriter writer(to, true, true);
			if (!writer.ok())
				return 1;

			auto res = append_content(from, writer);
			if (!res) return res;
			
			writer.close();
			return 0;
		}
		
		bool append_content(const std::string & from, fwriter & to)
		{
			freader reader(from, true);
			if (!reader.ok())
				return 1;

			reader.seek(0, seekdir::end);
			long size = reader.pos();
			reader.seek(0, seekdir::start);

			static long maxbuff = 1048576; // 1 MB
			std::string buffer;

			while (size > 0)
			{
				buffer.clear();
				long chunksize = std::min(size, maxbuff);
				reader.read(buffer, chunksize);
				to.write(buffer);
				size -= maxbuff;
			}

			reader.close();
			return 0;
		}

		////////////////////////////////////////////////////////////////////////////////

		time_t atime(const std::string & path)
		{
			struct stat attrib;
			stat(path.c_str(), &attrib);
			return (attrib.st_atime);
		}
		
		time_t mtime(const std::string & path)
		{
			struct stat attrib;
			stat(path.c_str(), &attrib);
			return (attrib.st_mtime);
		}
		
		time_t ctime(const std::string & path)
		{
			struct stat attrib;
			stat(path.c_str(), &attrib);
			return (attrib.st_ctime);
		}
	}
}