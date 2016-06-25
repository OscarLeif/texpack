// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef CPPTL_JSON_READER_H_INCLUDED
#define CPPTL_JSON_READER_H_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
#include "features.h"
#include "value.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <deque>
#include <iosfwd>
#include <stack>
#include <string>
#include <istream>

// Disable warning C4251: <data member>: <type> needs to have dll-interface to
// be used by...
#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

namespace json
{
	/** \brief Unserialize a <a HREF="http://www.json.org">JSON</a> document into a
	 *value.
	 *
	 * \deprecated Use char_reader and char_reader_builder.
	 */
	class JSON_API reader
	{
	public:
		typedef const char* location;

		/** \brief An error tagged with where in the JSON text it was encountered.
		 *
		 * The offsets give the [start, limit) range of bytes within the text. Note
		 * that this is bytes, not codepoints.
		 *
		 */
		struct structured_error
		{
			size_t offset_start;
			size_t offset_limit;
			std::string message;
		};

		/** \brief Constructs a Reader allowing all features
		 * for parsing.
		 */
		reader();

		/** \brief Constructs a Reader allowing the specified feature set
		 * for parsing.
		 */
		reader(const features & features);

		/** \brief Read a value from a <a HREF="http://www.json.org">JSON</a>
		 * document.
		 * \param document UTF-8 encoded string containing the document to read.
		 * \param root [out] Contains the root value of the document if it was
		 *             successfully parsed.
		 * \param collectComments \c true to collect comment and allow writing them
		 * back during
		 *                        serialization, \c false to discard comments.
		 *                        This parameter is ignored if
		 * Features::allowComments_
		 *                        is \c false.
		 * \return \c true if the document was successfully parsed, \c false if an
		 * error occurred.
		 */
		bool parse(const std::string& document, value& root, bool collectComments = true);

		/** \brief Read a value from a <a HREF="http://www.json.org">JSON</a>
		 document.
		 * \param beginDoc Pointer on the beginning of the UTF-8 encoded string of the
		 document to read.
		 * \param endDoc Pointer on the end of the UTF-8 encoded string of the
		 document to read.
		 *               Must be >= beginDoc.
		 * \param root [out] Contains the root value of the document if it was
		 *             successfully parsed.
		 * \param collectComments \c true to collect comment and allow writing them
		 back during
		 *                        serialization, \c false to discard comments.
		 *                        This parameter is ignored if
		 Features::allowComments_
		 *                        is \c false.
		 * \return \c true if the document was successfully parsed, \c false if an
		 error occurred.
		 */
		bool parse(const char* beginDoc, const char* endDoc, value& root, bool collectComments = true);

		/// \brief Parse from input stream.
		/// \see json::operator>>(std::istream&, json::value&).
		bool parse(std::istream& is, value& root, bool collectComments = true);

		/** \brief Returns a user friendly string that list errors in the parsed
		 * document.
		 * \return Formatted error message with the list of errors with their location
		 * in
		 *         the parsed document. An empty string is returned if no error
		 * occurred
		 *         during parsing.
		 * \deprecated Use get_formatted_error_messages() instead (typo fix).
		 */
		JSONCPP_DEPRECATED("Use get_formatted_error_messages() instead.")
		std::string get_formated_error_messages() const;

		/** \brief Returns a user friendly string that list errors in the parsed
		 * document.
		 * \return Formatted error message with the list of errors with their location
		 * in
		 *         the parsed document. An empty string is returned if no error
		 * occurred
		 *         during parsing.
		 */
		std::string get_formatted_error_messages() const;

		/** \brief Returns a vector of structured erros encounted while parsing.
		 * \return A (possibly empty) vector of structured_error objects. Currently
		 *         only one error can be returned, but the caller should tolerate
		 * multiple
		 *         errors.  This can occur if the parser recovers from a non-fatal
		 *         parse error and then encounters additional errors.
		 */
		std::vector<structured_error> get_structured_errors() const;

		/** \brief Add a semantic error message.
		 * \param value JSON value location associated with the error
		 * \param message The error message.
		 * \return \c true if the error was successfully added, \c false if the
		 * value offset exceeds the document size.
		 */
		bool push_error(const value & value, const std::string & message);

		/** \brief Add a semantic error message with extra context.
		 * \param value JSON value location associated with the error
		 * \param message The error message.
		 * \param extra Additional JSON value location to contextualize the error
		 * \return \c true if the error was successfully added, \c false if either
		 * value offset exceeds the document size.
		 */
		bool push_error(const value & value, const std::string & message, const json::value & extra);

		/** \brief Return whether there are any errors.
		 * \return \c true if there are no errors to report \c false if
		 * errors have occurred.
		 */
		bool good() const;

	private:
		enum token_type
		{
			token_end_of_stream = 0,
			token_object_begin,
			token_object_end,
			token_array_begin,
			token_array_end,
			token_string,
			token_number,
			token_true,
			token_false,
			token_null,
			token_array_separator,
			token_member_separator,
			token_comment,
			token_error
		};

		class token
		{
		public:
			token_type type_;
			location start_;
			location end_;
		};

		class error_info
		{
		public:
			token token_;
			std::string message_;
			location extra_;
		};

		typedef std::deque<error_info> errors;

		bool read_token(token & token);
		void skip_spaces();
		bool match(location pattern, int patternLength);
		bool read_comment();
		bool read_cstyle_comment();
		bool read_cpp_style_comment();
		bool read_string();
		void read_number();
		bool read_value();
		bool read_object(token & token);
		bool read_array(token & token);
		bool decode_number(token & token);
		bool decode_number(token & token, value & decoded);
		bool decode_string(token & token);
		bool decode_string(token & token, std::string & decoded);
		bool decode_double(token & token);
		bool decode_double(token & token, value & decoded);
		bool decode_unicode_code_point(token & token, location & current, location end, uint & unicode);
		bool decode_unicode_escape_sequence(token & token, location & current, location end, uint & unicode);
		bool add_error(const std::string & message, token & token, location extra = 0);
		bool recover_from_error(token_type skipUntilToken);
		bool add_error_and_recover(const std::string & message, token & token, token_type skipUntilToken);
		void skip_until_space();
		value& current_value();
		char get_next_char();
		void get_location_line_and_column(location location, int & line, int & column) const;
		std::string get_location_line_and_column(location location) const;
		void add_comment(location begin, location end, comment_placement placement);
		void skip_comment_tokens(token & token);

		typedef std::stack<value*> nodes;
		nodes nodes_;
		errors errors_;
		std::string document_;
		location begin_;
		location end_;
		location current_;
		location lastValueEnd_;
		value* lastValue_;
		std::string commentsBefore_;
		features features_;
		bool collectComments_;
	};  // Reader

	/** Interface for reading JSON from a char array.
	 */
	class JSON_API char_reader
	{
	public:
		virtual ~char_reader()
		{ }

		/** \brief Read a value from a <a HREF="http://www.json.org">JSON</a>
		 document.
		 * The document must be a UTF-8 encoded string containing the document to read.
		 *
		 * \param beginDoc Pointer on the beginning of the UTF-8 encoded string of the
		 document to read.
		 * \param endDoc Pointer on the end of the UTF-8 encoded string of the
		 document to read.
		 *        Must be >= beginDoc.
		 * \param root [out] Contains the root value of the document if it was
		 *             successfully parsed.
		 * \param errs [out] Formatted error messages (if not NULL)
		 *        a user friendly string that lists errors in the parsed
		 * document.
		 * \return \c true if the document was successfully parsed, \c false if an
		 error occurred.
		 */
		virtual bool parse(char const * beginDoc, char const * endDoc, value * root, std::string * errs) = 0;

		class JSON_API factory
		{
		public:
			virtual ~factory()
			{ }
			/** \brief Allocate a char_reader via operator new().
			 * \throw std::exception if something goes wrong (e.g. invalid settings)
			 */
			virtual char_reader* new_char_reader() const = 0;
		};  // factory
	};  // char_reader

	/** \brief Build a char_reader implementation.

	Usage:
	\code
	using namespace json;
	char_reader_builder builder;
	builder["collectComments"] = false;
	value value;
	std::string errs;
	bool ok = parseFromStream(builder, std::cin, &value, &errs);
	\endcode
	*/
	class JSON_API char_reader_builder : public char_reader::factory
	{
	public:
		// Note: We use a json::value so that we can add data-members to this class
		// without a major version bump.
		/** Configuration of this builder.
		  These are case-sensitive.
		  Available settings (case-sensitive):
		  - `"collectComments": false or true`
		  - true to collect comment and allow writing them
		  back during serialization, false to discard comments.
		  This parameter is ignored if allowComments is false.
		  - `"allowComments": false or true`
		  - true if comments are allowed.
		  - `"strictRoot": false or true`
		  - true if root must be either an array or an object value
		  - `"allowDroppedNullPlaceholders": false or true`
		  - true if dropped null placeholders are allowed. (See StreamWriterBuilder.)
		  - `"allowNumericKeys": false or true`
		  - true if numeric object keys are allowed.
		  - `"allowSingleQuotes": false or true`
		  - true if '' are allowed for strings (both keys and values)
		  - `"stackLimit": integer`
		  - Exceeding stackLimit (recursive depth of `read_value()`) will
		  cause an exception.
		  - This is a security issue (seg-faults caused by deeply nested JSON),
		  so the default is low.
		  - `"failIfExtra": false or true`
		  - If true, `parse()` returns false when extra non-whitespace trails
		  the JSON value in the input string.
		  - `"rejectDupKeys": false or true`
		  - If true, `parse()` returns false when a key is duplicated within an object.
		  - `"allowSpecialFloats": false or true`
		  - If true, special float values (NaNs and infinities) are allowed
		  and their values are lossfree restorable.

		  You can examine 'settings_` yourself
		  to see the defaults. You can also write and read them just like any
		  JSON value.
		  \sa set_defaults()
		  */
		json::value settings_;

		char_reader_builder();
		~char_reader_builder() override;

		char_reader* new_char_reader() const override;

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
		 * \snippet src/lib_json/json_reader.cpp CharReaderBuilderDefaults
		 */
		static void set_defaults(json::value* settings);
		/** Same as old Features::strict_mode().
		 * \pre 'settings' != NULL (but json::null is fine)
		 * \remark Defaults:
		 * \snippet src/lib_json/json_reader.cpp CharReaderBuilderStrictMode
		 */
		static void strict_mode(json::value* settings);
	};

	/** Consume entire stream and use its begin/end.
	  * Someday we might have a real StreamReader, but for now this
	  * is convenient.
	  */
	bool JSON_API parseFromStream(char_reader::factory const&, std::istream&, value* root, std::string* errs);

	/** \brief Read from 'sin' into 'root'.

	 Always keep comments from the input JSON.

	 This can be used to read a file into a particular sub-object.
	 For example:
	 \code
	 json::value root;
	 cin >> root["dir"]["file"];
	 cout << root;
	 \endcode
	 Result:
	 \verbatim
	 {
	 "dir": {
	 "file": {
	 // The input stream JSON would be nested here.
	 }
	 }
	 }
	 \endverbatim
	 \throw std::exception on parse error.
	 \see json::operator<<()
	 */
	JSON_API std::istream& operator>>(std::istream&, value&);
} // namespace json

#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(pop)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

#endif // CPPTL_JSON_READER_H_INCLUDED
