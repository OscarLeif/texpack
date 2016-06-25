// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef JSON_WRITER_H_INCLUDED
#define JSON_WRITER_H_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
#include "value.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <vector>
#include <string>
#include <ostream>

// Disable warning C4251: <data member>: <type> needs to have dll-interface to
// be used by...
#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

namespace json
{
	class value;

	/**

	Usage:
	\code
	using namespace json;
	void writeToStdout(stream_writer::factory const& factory, value const& value) {
	std::unique_ptr<stream_writer> const writer(
	factory.new_stream_writer());
	writer->write(value, &std::cout);
	std::cout << std::endl;  // add lf and flush
	}
	\endcode
	*/
	class JSON_API stream_writer
	{
	protected:
		std::ostream* sout_;  // not owned; will not delete

	public:
		stream_writer();
		virtual ~stream_writer();
		/** Write value into document as configured in sub-class.
			Do not take ownership of sout, but maintain a reference during function.
			\pre sout != NULL
			\return zero on success (For now, we always return zero, so check the stream instead.)
			\throw std::exception possibly, depending on configuration
			*/
		virtual int write(value const& root, std::ostream* sout) = 0;

		/** \brief A simple abstract factory.
		 */
		class JSON_API factory
		{
		public:
			virtual ~factory();
			/** \brief Allocate a CharReader via operator new().
			 * \throw std::exception if something goes wrong (e.g. invalid settings)
			 */
			virtual stream_writer* new_stream_writer() const = 0;
		};  // factory
	};  // stream_writer

	/** \brief Write into stringstream, then return string, for convenience.
	 * A stream_writer will be created from the factory, used, and then deleted.
	 */
	std::string JSON_API write_string(stream_writer::factory const& factory, value const& root);

	/** \brief Build a stream_writer implementation.

	Usage:
	\code
	using namespace json;
	value value = ...;
	stream_writer_builder builder;
	builder["commentStyle"] = "None";
	builder["indentation"] = "   ";  // or whatever you like
	std::unique_ptr<json::stream_writer> writer(
	builder.new_stream_writer());
	writer->write(value, &std::cout);
	std::cout << std::endl;  // add lf and flush
	\endcode
	*/
	class JSON_API stream_writer_builder : public stream_writer::factory
	{
	public:
		// Note: We use a json::value so that we can add data-members to this class
		// without a major version bump.
		/** Configuration of this builder.
		  Available settings (case-sensitive):
		  - "commentStyle": "None" or "All"
		  - "indentation":  "<anything>"
		  - "enable_YAML_compatibility": false or true
		  - slightly change the whitespace around colons
		  - "dropNullPlaceholders": false or true
		  - Drop the "null" string from the writer's output for nullValues.
		  Strictly speaking, this is not valid JSON. But when the output is being
		  fed to a browser's Javascript, it makes for smaller output and the
		  browser can handle the output just fine.
		  - "useSpecialFloats": false or true
		  - If true, outputs non-finite floating point values in the following way:
		  NaN values as "NaN", positive infinity as "Infinity", and negative infinity
		  as "-Infinity".

		  You can examine 'settings_` yourself
		  to see the defaults. You can also write and read them just like any
		  JSON value.
		  \sa set_defaults()
		  */
		json::value settings_;

		stream_writer_builder();
		~stream_writer_builder() override;

		/**
		 * \throw std::exception if something goes wrong (e.g. invalid settings)
		 */
		stream_writer* new_stream_writer() const override;

		/** \return true if 'settings' are legal and consistent;
		 *   otherwise, indicate bad settings via 'invalid'.
		 */
		bool validate(json::value* invalid) const;
		/** A simple way to update a specific setting.
		 */
		value& operator[](std::string key);

		/** Called by ctor, but you can use this to reset settings_.
		 * \pre 'settings' != NULL (but json::null is fine)
		 * \remark Defaults:
		 * \snippet src/lib_json/json_writer.cpp StreamWriterBuilderDefaults
		 */
		static void set_defaults(json::value* settings);
	};

	/** \brief Abstract class for writers.
	 * \deprecated Use stream_writer. (And really, this is an implementation detail.)
	 */
	class JSON_API writer
	{
	public:
		virtual ~writer();

		virtual std::string write(const value& root) = 0;
	};

	/** \brief Outputs a value in <a HREF="http://www.json.org">JSON</a> format
	 *without formatting (not human friendly).
	 *
	 * The JSON document is written in a single line. It is not intended for 'human'
	 *consumption,
	 * but may be usefull to support feature such as RPC where bandwith is limited.
	 * \sa Reader, value
	 * \deprecated Use stream_writer_builder.
	 */
	class JSON_API fast_writer : public writer
	{
	public:
		fast_writer();
		~fast_writer() override {}

		void enable_YAML_compatibility();

		/** \brief Drop the "null" string from the writer's output for nullValues.
		 * Strictly speaking, this is not valid JSON. But when the output is being
		 * fed to a browser's Javascript, it makes for smaller output and the
		 * browser can handle the output just fine.
		 */
		void dropNullPlaceholders();

		void omitEndingLineFeed();

	public: // overridden from writer
		std::string write(const value& root) override;

	private:
		void write_value(const value& value);

		std::string document_;
		bool yamlCompatiblityEnabled_;
		bool dropNullPlaceholders_;
		bool omitEndingLineFeed_;
	};

	/** \brief Writes a value in <a HREF="http://www.json.org">JSON</a> format in a
	 *human friendly way.
	 *
	 * The rules for line break and indent are as follow:
	 * - Object value:
	 *     - if empty then print {} without indent and line break
	 *     - if not empty the print '{', line break & indent, print one value per
	 *line
	 *       and then unindent and line break and print '}'.
	 * - Array value:
	 *     - if empty then print [] without indent and line break
	 *     - if the array contains no object value, empty array or some other value
	 *types,
	 *       and all the values fit on one lines, then print the array on a single
	 *line.
	 *     - otherwise, it the values do not fit on one line, or the array contains
	 *       object or non empty array, then print one value per line.
	 *
	 * If the value have comments then they are outputed according to their
	 *#CommentPlacement.
	 *
	 * \sa Reader, value, value::setComment()
	 * \deprecated Use stream_writer_builder.
	 */
	class JSON_API styled_writer : public writer
	{
	public:
		styled_writer();
		~styled_writer() override {}

	public: // overridden from writer
		/** \brief Serialize a value in <a HREF="http://www.json.org">JSON</a> format.
		 * \param root value to serialize.
		 * \return String containing the JSON document that represents the root value.
		 */
		std::string write(const value& root) override;

	private:
		void write_value(const value& value);
		void write_array_value(const value& value);
		bool is_multine_array(const value& value);
		void push_value(const std::string& value);
		void write_indent();
		void write_with_indent(const std::string& value);
		void indent();
		void unindent();
		void write_comment_before_value(const value& root);
		void write_comment_after_value_on_same_line(const value& root);
		bool has_comment_for_value(const value& value);
		static std::string normalize_eol(const std::string& text);

		typedef std::vector<std::string> ChildValues;

		ChildValues childValues_;
		std::string document_;
		std::string indentString_;
		int rightMargin_;
		int indentSize_;
		bool addChildValues_;
	};

	/** \brief Writes a value in <a HREF="http://www.json.org">JSON</a> format in a
	 human friendly way,
	 to a stream rather than to a string.
	 *
	 * The rules for line break and indent are as follow:
	 * - Object value:
	 *     - if empty then print {} without indent and line break
	 *     - if not empty the print '{', line break & indent, print one value per
	 line
	 *       and then unindent and line break and print '}'.
	 * - Array value:
	 *     - if empty then print [] without indent and line break
	 *     - if the array contains no object value, empty array or some other value
	 types,
	 *       and all the values fit on one lines, then print the array on a single
	 line.
	 *     - otherwise, it the values do not fit on one line, or the array contains
	 *       object or non empty array, then print one value per line.
	 *
	 * If the value have comments then they are outputed according to their
	 #CommentPlacement.
	 *
	 * \param indentation Each level will be indented by this amount extra.
	 * \sa Reader, value, value::setComment()
	 * \deprecated Use stream_writer_builder.
	 */
	class JSON_API styled_stream_writer
	{
	public:
		styled_stream_writer(std::string indentation = "\t");
		~styled_stream_writer()
		{ }

	public:
		/** \brief Serialize a value in <a HREF="http://www.json.org">JSON</a> format.
		 * \param out Stream to write to. (Can be ostringstream, e.g.)
		 * \param root value to serialize.
		 * \note There is no point in deriving from writer, since write() should not
		 * return a value.
		 */
		void write(std::ostream& out, const value& root);

	private:
		void write_value(const value& value);
		void write_array_value(const value& value);
		bool is_multine_array(const value& value);
		void push_value(const std::string& value);
		void write_indent();
		void write_with_indent(const std::string& value);
		void indent();
		void unindent();
		void write_comment_before_value(const value& root);
		void write_comment_after_value_on_same_line(const value& root);
		bool has_comment_for_value(const value& value);
		static std::string normalize_eol(const std::string& text);

		typedef std::vector<std::string> ChildValues;

		ChildValues childValues_;
		std::ostream* document_;
		std::string indentString_;
		int rightMargin_;
		std::string indentation_;
		bool addChildValues_ : 1;
		bool indented_ : 1;
	};

#if defined(JSON_HAS_INT64)
	std::string JSON_API valueToString(int value);
	std::string JSON_API valueToString(uint value);
#endif // if defined(JSON_HAS_INT64)
	std::string JSON_API valueToString(largest_int value);
	std::string JSON_API valueToString(largest_uint value);
	std::string JSON_API valueToString(double value);
	std::string JSON_API valueToString(bool value);
	std::string JSON_API valueToQuotedString(const char* value);

	/// \brief Output using the styled_stream_writer.
	/// \see json::operator>>()
	JSON_API std::ostream& operator<<(std::ostream&, const value& root);
} // namespace json

#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(pop)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

#endif // JSON_WRITER_H_INCLUDED
