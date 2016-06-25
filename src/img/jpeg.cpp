#include "jpeg.hpp"

#include <stdio.h>
#include <jpeglib.h>
#include <jerror.h>

#define INPUT_BUF_SIZE  4096
#define OUTPUT_BUF_SIZE  4096

namespace
{
	struct core_error_mgr
	{
		struct jpeg_error_mgr pub;	/* "public" fields */
		jmp_buf setjmp_buffer;	/* for return to caller */
	};

	using core_error_ptr = core_error_mgr *;

	inline void JPEG_EXIT(j_common_ptr cinfo, int code)
	{
		core_error_ptr error_ptr = (core_error_ptr)cinfo->err;
		error_ptr->pub.msg_code = code;
		error_ptr->pub.error_exit(cinfo);
	}

	inline void JPEG_WARNING(j_common_ptr cinfo, int code)
	{
		core_error_ptr error_ptr = (core_error_ptr)cinfo->err;
		error_ptr->pub.msg_code = code;
		error_ptr->pub.emit_message(cinfo, -1);
	}

	static void my_error_exit(j_common_ptr cinfo)
	{
		core_error_ptr myerr = (core_error_ptr)cinfo->err;
		(*cinfo->err->output_message) (cinfo);
		longjmp(myerr->setjmp_buffer, 1);
	}

	void my_output_message(j_common_ptr info)
	{
		char buffer[JMSG_LENGTH_MAX];
		((core_error_ptr)info)->pub.format_message(info, buffer);
		printf("[JPEG] %s\n", buffer);
	}

	////////////////////////////////////////////

	struct core_src
	{
		/// public fields
		struct jpeg_source_mgr pub;
		/// source stream
		void * infile;
		core::freader * m_io;
		/// start of buffer
		JOCTET * buffer;
		/// have we gotten any data yet ?
		boolean start_of_file;
	};

	using core_src_ptr = core_src *;

	void init_source(j_decompress_ptr cinfo)
	{
		core_src_ptr src = (core_src_ptr)cinfo->src;
		src->start_of_file = TRUE;
		/* We reset the empty-input-file flag for each image,
		* but we don't clear the input buffer.
		* This is correct behavior for reading a series of images from one source. */
	}

	boolean fill_input_buffer(j_decompress_ptr cinfo)
	{
		core_src_ptr src = (core_src_ptr)cinfo->src;
		size_t nbytes = src->m_io->read(src->buffer, 1, INPUT_BUF_SIZE);// , src->infile); TODO - check
		if (nbytes <= 0)
		{
			if (src->start_of_file)
			{
				//treat empty input file as fatal error

				//let the memory manager delete any temp files before we die
				jpeg_destroy((j_common_ptr)cinfo);
				JPEG_EXIT((j_common_ptr)cinfo, JERR_INPUT_EMPTY);
			}
			JPEG_WARNING((j_common_ptr)cinfo, JWRN_JPEG_EOF);

			//Insert a fake EOI marker
			src->buffer[0] = (JOCTET)0xFF;
			src->buffer[1] = (JOCTET)JPEG_EOI;
			nbytes = 2;
		}

		src->pub.next_input_byte = src->buffer;
		src->pub.bytes_in_buffer = nbytes;
		src->start_of_file = FALSE;
		return TRUE;
	}

	void skip_input_data(j_decompress_ptr info, long bytescnt)
	{
		//Just a dumb implementation for now.  Could use fseek() except
		//it doesn't work on pipes.  Not clear that being smart is worth
		//any trouble anyway --- large skips are infrequent
		core_src_ptr src = (core_src_ptr)info->src;
		if (bytescnt > 0)
		{
			while (bytescnt > (long)src->pub.bytes_in_buffer)
			{
				bytescnt -= (long)src->pub.bytes_in_buffer;
				(void)fill_input_buffer(info);
				//note we assume that fill_input_buffer will never return FALSE,
				//so suspension need not be handled
			}

			src->pub.next_input_byte += (size_t)bytescnt;
			src->pub.bytes_in_buffer -= (size_t)bytescnt;
		}
	}

	void term_source(j_decompress_ptr info) { }

	void jpeg_core_src(j_decompress_ptr info, core::freader & reader)
	{
		core_src_ptr src;

		// allocate memory for the buffer. is released automatically in the end

		if (info->src == nullptr)
		{
			info->src = (struct jpeg_source_mgr *) (*info->mem->alloc_small)
				((j_common_ptr)info, JPOOL_PERMANENT, sizeof(core_src));

			src = (core_src_ptr)info->src;

			src->buffer = (JOCTET *)(*info->mem->alloc_small)
				((j_common_ptr)info, JPOOL_PERMANENT, INPUT_BUF_SIZE * sizeof(JOCTET));
		}

		// initialize the jpeg pointer struct with pointers to functions

		src = (core_src_ptr)info->src;
		src->pub.init_source = init_source;
		src->pub.fill_input_buffer = fill_input_buffer;
		src->pub.skip_input_data = skip_input_data;
		src->pub.resync_to_restart = jpeg_resync_to_restart; // use default method 
		src->pub.term_source = term_source;
		src->infile = nullptr; //infile; TODO - check
		src->m_io = &reader;
		src->pub.bytes_in_buffer = 0;		// forces fill_input_buffer on first read 
		src->pub.next_input_byte = NULL;	// until buffer loaded 
	}

	////////////////////////////////////////////

	struct core_dst
	{
		/// public fields
		struct jpeg_destination_mgr pub;
		/// destination stream
		void * outfile;
		core::fwriter * m_io;
		/// start of buffer
		JOCTET * buffer;
	};

	using core_dst_ptr = core_dst *;

	void init_destination(j_compress_ptr info)
	{
		core_dst_ptr dest = (core_dst_ptr)info->dest;

		dest->buffer = (JOCTET *)
			(*info->mem->alloc_small) ((j_common_ptr)info,
			JPOOL_IMAGE,
			OUTPUT_BUF_SIZE * sizeof(JOCTET));

		dest->pub.next_output_byte = dest->buffer;
		dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
	}

	boolean empty_output_buffer(j_compress_ptr info)
	{
		core_dst_ptr dest = (core_dst_ptr)info->dest;
		if (dest->m_io->write(dest->buffer, OUTPUT_BUF_SIZE) != OUTPUT_BUF_SIZE)
		{
			// let the memory manager delete any temp files before we die
			jpeg_destroy((j_common_ptr)info);
			JPEG_EXIT((j_common_ptr)info, JERR_FILE_WRITE);
		}

		dest->pub.next_output_byte = dest->buffer;
		dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
		return TRUE;
	}

	void term_destination(j_compress_ptr info)
	{
		core_dst_ptr dest = (core_dst_ptr)info->dest;
		size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;
		// write any data remaining in the buffer
		if (datacount <= 0)
			return;
		
		if (dest->m_io->write(dest->buffer, (unsigned int)datacount) != datacount)
		{
			// let the memory manager delete any temp files before we die
			jpeg_destroy((j_common_ptr)info);
			JPEG_EXIT((j_common_ptr)info, JERR_FILE_WRITE);
		}
	}

	void jpeg_core_dest(j_compress_ptr info, core::fwriter & writer)
	{
		core_dst_ptr dest;
		if (info->dest == nullptr)
		{
			info->dest = (struct jpeg_destination_mgr *)(*info->mem->alloc_small)
				((j_common_ptr)info, JPOOL_PERMANENT, sizeof(core_dst));
		}

		dest = (core_dst_ptr)info->dest;
		dest->pub.init_destination = init_destination;
		dest->pub.empty_output_buffer = empty_output_buffer;
		dest->pub.term_destination = term_destination;
		dest->outfile = nullptr;
		dest->m_io = &writer;
	}
}

namespace img
{
	jpeg::jpeg() : img(), quality(100) { }

	jpeg::jpeg(const std::string & fname)
		: img()
	{
		load(fname);
	}

	jpeg::jpeg(unsigned width, unsigned height)
		: img(width, height)
		, quality(100)
	{ }

	////////////////////////////////////////////////////////////

	void jpeg::save(const std::string & fname)
	{
		//Check if data is present
		if (_data == nullptr)
			throw std::exception("data isn't present");

		auto outfile = core::io::write(fname.c_str(), true, false);
		if (!outfile.opened() || !outfile.ok()) throw std::exception("cant open file");

		struct jpeg_compress_struct info;
		struct core_error_mgr jerr;

		info.err = jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = my_error_exit;
		jerr.pub.output_message = my_output_message;
		jpeg_create_compress(&info);

		//jpeg_stdio_dest(&info, outfile);
		jpeg_core_dest(&info, outfile);

		info.image_width = _w;
		info.image_height = _h;
		info.input_components = 3;
		info.in_color_space = JCS_RGB;

		jpeg_set_defaults(&info);

		info.write_Adobe_marker = FALSE;
		info.write_JFIF_header = FALSE;

		info.comp_info[0].h_samp_factor = 4;// Y
		info.comp_info[0].v_samp_factor = 1;
		info.comp_info[1].h_samp_factor = 1;// Cb
		info.comp_info[1].v_samp_factor = 1;
		info.comp_info[2].h_samp_factor = 1;// Cr
		info.comp_info[2].v_samp_factor = 1;

		jpeg_set_quality(&info, quality, TRUE /* limit to baseline-JPEG values */);

		jpeg_start_compress(&info, TRUE);

		byte * buffer = new byte[_w * 3];
		unsigned y = 0;
		while (info.next_scanline < info.image_height)
		{
			for (unsigned x = 0; x < _w; ++x)
			{
				unsigned idx = y*_w + x;
				buffer[x * 3 + 0] = _data[idx].r;
				buffer[x * 3 + 1] = _data[idx].g;
				buffer[x * 3 + 2] = _data[idx].b;
			}

			jpeg_write_scanlines(&info, &buffer, 1);
			++y;
		}
		delete[] buffer;

		jpeg_finish_compress(&info);
		jpeg_destroy_compress(&info);
		outfile.close();
	}

	////////////////////////////////////////////////////////////

	void jpeg::load(const std::string & fname)
	{
		//Clean up old data
		if (_data != nullptr)
			delete[] _data;

		auto reader = core::io::read(fname, true);
		if (!reader.opened() || !reader.ok())
			throw std::exception("cant open file");

		load(reader);

		//close file
		reader.close();
	}

	void jpeg::load(core::freader & reader)
	{
		struct jpeg_decompress_struct info;
		struct core_error_mgr jerr;

		JSAMPARRAY buffer;
		int row_stride;

		info.err = jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = my_error_exit;
		jerr.pub.output_message = &my_output_message;

		if (setjmp(jerr.setjmp_buffer))
		{
			jpeg_destroy_decompress(&info);
			throw std::exception("JPEG error");
		}

		//init decompress object
		jpeg_create_decompress(&info);
		//specify data source (file reader)
		jpeg_core_src(&info, reader);
		//read header
		jpeg_read_header(&info, TRUE);
		//start decompression
		jpeg_start_decompress(&info);

		//stride
		row_stride = info.output_width * info.output_components;
		auto alloc = info.mem->alloc_sarray;
		buffer = (*alloc) ((j_common_ptr)&info, JPOOL_IMAGE, row_stride, 1);

		//alloc data
		_w = info.image_width;
		_h = info.image_height;
		_data = new color[_w*_h];
		int components = info.output_components;

		//read scan lines
		int y = 0;
		while (info.output_scanline < info.output_height)
		{
			jpeg_read_scanlines(&info, buffer, 1);

			for (unsigned x = 0; x < _w; ++x)
			{
				unsigned idx = y*_w + x;
				_data[idx].r = buffer[0][x*components + 0];
				_data[idx].g = buffer[0][x*components + 1];
				_data[idx].b = buffer[0][x*components + 2];
				_data[idx].a = 255;
			}

			++y;
		}

		//stop decompression
		jpeg_finish_decompress(&info);
		//destroy info
		jpeg_destroy_decompress(&info);
	}
}