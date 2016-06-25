// Copyright 2007-2011 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
#include <json/assertions.h>
#include <json/reader.h>
#include <json/value.h>
#include "json_tool.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <utility>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <istream>
#include <sstream>
#include <memory>
#include <set>
#include <limits>

#if defined(_MSC_VER)
#if !defined(WINCE) && defined(__STDC_SECURE_LIB__) && _MSC_VER >= 1500 // VC++ 9.0 and above 
#define snprintf sprintf_s
#elif _MSC_VER >= 1900 // VC++ 14.0 and above
#define snprintf std::snprintf
#else
#define snprintf _snprintf
#endif
#elif defined(__ANDROID__) || defined(__QNXNTO__)
#define snprintf snprintf
#elif __cplusplus >= 201103L
#define snprintf std::snprintf
#endif

#if defined(__QNXNTO__)
#define sscanf std::sscanf
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400 // VC++ 8.0
// Disable warning about strdup being deprecated.
#pragma warning(disable : 4996)
#endif

static int const stackLimit_g = 1000;
static int       stackDepth_g = 0;  // see read_value()

namespace json {

#if __cplusplus >= 201103L || (defined(_CPPLIB_VER) && _CPPLIB_VER >= 520)
	typedef std::unique_ptr<char_reader> char_reader_ptr;
#else
	typedef std::auto_ptr<char_reader>   char_reader_ptr;
#endif

	// Implementation of class features
	// ////////////////////////////////

	features::features()
		: allow_comments_(true)
		, strict_root_(false)
		, allow_dropped_null_placeholders_(false)
		, allow_numeric_keys_(false)
	{ }

	features features::all() { return features(); }

	features features::strict_mode()
	{
		features features;
		features.allow_comments_ = false;
		features.strict_root_ = true;
		features.allow_dropped_null_placeholders_ = false;
		features.allow_numeric_keys_ = false;
		return features;
	}

	// Implementation of class reader
	// ////////////////////////////////

	static bool containsNewLine(reader::location begin, reader::location end)
	{
		for (; begin < end; ++begin)
			if (*begin == '\n' || *begin == '\r')
				return true;

		return false;
	}

	// Class reader
	// //////////////////////////////////////////////////////////////////

	reader::reader()
		: errors_()
		, document_()
		, begin_()
		, end_()
		, current_()
		, lastValueEnd_()
		, lastValue_()
		, commentsBefore_()
		, features_(features::all())
		, collectComments_()
	{ }

	reader::reader(const features& features)
		: errors_()
		, document_()
		, begin_()
		, end_()
		, current_()
		, lastValueEnd_()
		, lastValue_()
		, commentsBefore_()
		, features_(features)
		, collectComments_()
	{ }

	bool reader::parse(const std::string& document, value& root, bool collectComments)
	{
		document_ = document;
		const char* begin = document_.c_str();
		const char* end = begin + document_.length();
		return parse(begin, end, root, collectComments);
	}

	bool reader::parse(std::istream& sin, value& root, bool collectComments)
	{
		// std::istream_iterator<char> begin(sin);
		// std::istream_iterator<char> end;
		// Those would allow streamed input from a file, if parse() were a
		// template function.

		// Since std::string is reference-counted, this at least does not
		// create an extra copy.
		std::string doc;
		std::getline(sin, doc, (char)EOF);
		return parse(doc, root, collectComments);
	}

	bool reader::parse(const char* beginDoc, const char* endDoc, value& root, bool collectComments)
	{
		if (!features_.allow_comments_) {
			collectComments = false;
		}

		begin_ = beginDoc;
		end_ = endDoc;
		collectComments_ = collectComments;
		current_ = begin_;
		lastValueEnd_ = 0;
		lastValue_ = 0;
		commentsBefore_ = "";
		errors_.clear();
		while (!nodes_.empty())
			nodes_.pop();
		nodes_.push(&root);

		stackDepth_g = 0;  // Yes, this is bad coding, but options are limited.
		bool successful = read_value();
		token token;
		skip_comment_tokens(token);
		if (collectComments_ && !commentsBefore_.empty())
			root.set_comment(commentsBefore_, comment_after);
		if (features_.strict_root_) {
			if (!root.is_array() && !root.is_object()) {
				// Set error location to start of doc, ideally should be first token found
				// in doc
				token.type_ = token_error;
				token.start_ = beginDoc;
				token.end_ = endDoc;
				add_error("A valid JSON document must be either an array or an object value.", token);
				return false;
			}
		}
		return successful;
	}

	bool reader::read_value() {
		// This is a non-reentrant way to support a stackLimit. Terrible!
		// But this deprecated class has a security problem: Bad input can
		// cause a seg-fault. This seems like a fair, binary-compatible way
		// to prevent the problem.
		if (stackDepth_g >= stackLimit_g) throw_runtime_error("Exceeded stackLimit in read_value().");
		++stackDepth_g;

		token token;
		skip_comment_tokens(token);
		bool successful = true;

		if (collectComments_ && !commentsBefore_.empty()) {
			current_value().set_comment(commentsBefore_, comment_before);
			commentsBefore_ = "";
		}

		switch (token.type_) {
		case token_object_begin:
			successful = read_object(token);
			current_value().set_offset_limit(current_ - begin_);
			break;
		case token_array_begin:
			successful = read_array(token);
			current_value().set_offset_limit(current_ - begin_);
			break;
		case token_number:
			successful = decode_number(token);
			break;
		case token_string:
			successful = decode_string(token);
			break;
		case token_true:
		{
			value v(true);
			current_value().swap_payload(v);
			current_value().set_offset_start(token.start_ - begin_);
			current_value().set_offset_limit(token.end_ - begin_);
		}
		break;
		case token_false:
		{
			value v(false);
			current_value().swap_payload(v);
			current_value().set_offset_start(token.start_ - begin_);
			current_value().set_offset_limit(token.end_ - begin_);
		}
		break;
		case token_null:
		{
			value v;
			current_value().swap_payload(v);
			current_value().set_offset_start(token.start_ - begin_);
			current_value().set_offset_limit(token.end_ - begin_);
		}
		break;
		case token_array_separator:
		case token_object_end:
		case token_array_end:
			if (features_.allow_dropped_null_placeholders_)
			{
				// "Un-read" the current token and mark the current value as a null
				// token.
				current_--;
				value v;
				current_value().swap_payload(v);
				current_value().set_offset_start(current_ - begin_ - 1);
				current_value().set_offset_limit(current_ - begin_);
				break;
			} // Else, fall through...
		default:
			current_value().set_offset_start(token.start_ - begin_);
			current_value().set_offset_limit(token.end_ - begin_);
			return add_error("Syntax error: value, object or array expected.", token);
		}

		if (collectComments_) {
			lastValueEnd_ = current_;
			lastValue_ = &current_value();
		}

		--stackDepth_g;
		return successful;
	}

	void reader::skip_comment_tokens(token& token) {
		if (features_.allow_comments_) {
			do {
				read_token(token);
			} while (token.type_ == token_comment);
		}
		else {
			read_token(token);
		}
	}

	bool reader::read_token(token& token) {
		skip_spaces();
		token.start_ = current_;
		char c = get_next_char();
		bool ok = true;
		switch (c) {
		case '{':
			token.type_ = token_object_begin;
			break;
		case '}':
			token.type_ = token_object_end;
			break;
		case '[':
			token.type_ = token_array_begin;
			break;
		case ']':
			token.type_ = token_array_end;
			break;
		case '"':
			token.type_ = token_string;
			ok = read_string();
			break;
		case '/':
			token.type_ = token_comment;
			ok = read_comment();
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '-':
			token.type_ = token_number;
			read_number();
			break;
		case 't':
			token.type_ = token_true;
			ok = match("rue", 3);
			break;
		case 'f':
			token.type_ = token_false;
			ok = match("alse", 4);
			break;
		case 'n':
			token.type_ = token_null;
			ok = match("ull", 3);
			break;
		case ',':
			token.type_ = token_array_separator;
			break;
		case ':':
			token.type_ = token_member_separator;
			break;
		case 0:
			token.type_ = token_end_of_stream;
			break;
		default:
			ok = false;
			break;
		}
		if (!ok)
			token.type_ = token_error;
		token.end_ = current_;
		return true;
	}

	void reader::skip_spaces() {
		while (current_ != end_) {
			char c = *current_;
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
				++current_;
			else
				break;
		}
	}

	bool reader::match(location pattern, int patternLength) {
		if (end_ - current_ < patternLength)
			return false;
		int index = patternLength;
		while (index--)
			if (current_[index] != pattern[index])
				return false;
		current_ += patternLength;
		return true;
	}

	bool reader::read_comment() {
		location commentBegin = current_ - 1;
		char c = get_next_char();
		bool successful = false;
		if (c == '*')
			successful = read_cstyle_comment();
		else if (c == '/')
			successful = read_cpp_style_comment();
		if (!successful)
			return false;

		if (collectComments_) {
			comment_placement placement = comment_before;
			if (lastValueEnd_ && !containsNewLine(lastValueEnd_, commentBegin)) {
				if (c != '*' || !containsNewLine(commentBegin, current_))
					placement = comment_after_on_same_line;
			}

			add_comment(commentBegin, current_, placement);
		}
		return true;
	}

	static std::string normalizeEOL(reader::location begin, reader::location end) {
		std::string normalized;
		normalized.reserve(end - begin);
		reader::location current = begin;
		while (current != end) {
			char c = *current++;
			if (c == '\r') {
				if (current != end && *current == '\n')
					// convert dos EOL
					++current;
				// convert Mac EOL
				normalized += '\n';
			}
			else {
				normalized += c;
			}
		}
		return normalized;
	}

	void
		reader::add_comment(location begin, location end, comment_placement placement) {
		assert(collectComments_);
		const std::string& normalized = normalizeEOL(begin, end);
		if (placement == comment_after_on_same_line) {
			assert(lastValue_ != 0);
			lastValue_->set_comment(normalized, placement);
		}
		else {
			commentsBefore_ += normalized;
		}
	}

	bool reader::read_cstyle_comment() {
		while (current_ != end_) {
			char c = get_next_char();
			if (c == '*' && *current_ == '/')
				break;
		}
		return get_next_char() == '/';
	}

	bool reader::read_cpp_style_comment() {
		while (current_ != end_) {
			char c = get_next_char();
			if (c == '\n')
				break;
			if (c == '\r') {
				// Consume DOS EOL. It will be normalized in add_comment.
				if (current_ != end_ && *current_ == '\n')
					get_next_char();
				// Break on Moc OS 9 EOL.
				break;
			}
		}
		return true;
	}

	void reader::read_number() {
		const char *p = current_;
		char c = '0'; // stopgap for already consumed character
		// integral part
		while (c >= '0' && c <= '9')
			c = (current_ = p) < end_ ? *p++ : 0;
		// fractional part
		if (c == '.') {
			c = (current_ = p) < end_ ? *p++ : 0;
			while (c >= '0' && c <= '9')
				c = (current_ = p) < end_ ? *p++ : 0;
		}
		// exponential part
		if (c == 'e' || c == 'E') {
			c = (current_ = p) < end_ ? *p++ : 0;
			if (c == '+' || c == '-')
				c = (current_ = p) < end_ ? *p++ : 0;
			while (c >= '0' && c <= '9')
				c = (current_ = p) < end_ ? *p++ : 0;
		}
	}

	bool reader::read_string() {
		char c = 0;
		while (current_ != end_) {
			c = get_next_char();
			if (c == '\\')
				get_next_char();
			else if (c == '"')
				break;
		}
		return c == '"';
	}

	bool reader::read_object(token& tokenStart) {
		token tokenName;
		std::string name;
		value init(object_value);
		current_value().swap_payload(init);
		current_value().set_offset_start(tokenStart.start_ - begin_);
		while (read_token(tokenName)) {
			bool initialTokenOk = true;
			while (tokenName.type_ == token_comment && initialTokenOk)
				initialTokenOk = read_token(tokenName);
			if (!initialTokenOk)
				break;
			if (tokenName.type_ == token_object_end && name.empty()) // empty object
				return true;
			name = "";
			if (tokenName.type_ == token_string) {
				if (!decode_string(tokenName, name))
					return recover_from_error(token_object_end);
			}
			else if (tokenName.type_ == token_number && features_.allow_numeric_keys_) {
				value numberName;
				if (!decode_number(tokenName, numberName))
					return recover_from_error(token_object_end);
				name = numberName.as_string();
			}
			else {
				break;
			}

			token colon;
			if (!read_token(colon) || colon.type_ != token_member_separator) {
				return add_error_and_recover(
					"Missing ':' after object member name", colon, token_object_end);
			}
			value& value = current_value()[name];
			nodes_.push(&value);
			bool ok = read_value();
			nodes_.pop();
			if (!ok) // error already set
				return recover_from_error(token_object_end);

			token comma;
			if (!read_token(comma) ||
				(comma.type_ != token_object_end && comma.type_ != token_array_separator &&
				comma.type_ != token_comment)) {
				return add_error_and_recover(
					"Missing ',' or '}' in object declaration", comma, token_object_end);
			}
			bool finalizeTokenOk = true;
			while (comma.type_ == token_comment && finalizeTokenOk)
				finalizeTokenOk = read_token(comma);
			if (comma.type_ == token_object_end)
				return true;
		}
		return add_error_and_recover(
			"Missing '}' or object member name", tokenName, token_object_end);
	}

	bool reader::read_array(token& tokenStart) {
		value init(array_value);
		current_value().swap_payload(init);
		current_value().set_offset_start(tokenStart.start_ - begin_);
		skip_spaces();
		if (*current_ == ']') // empty array
		{
			token endArray;
			read_token(endArray);
			return true;
		}
		int index = 0;
		for (;;) {
			value& value = current_value()[index++];
			nodes_.push(&value);
			bool ok = read_value();
			nodes_.pop();
			if (!ok) // error already set
				return recover_from_error(token_array_end);

			token token;
			// Accept Comment after last item in the array.
			ok = read_token(token);
			while (token.type_ == token_comment && ok) {
				ok = read_token(token);
			}
			bool badTokenType =
				(token.type_ != token_array_separator && token.type_ != token_array_end);
			if (!ok || badTokenType) {
				return add_error_and_recover(
					"Missing ',' or ']' in array declaration", token, token_array_end);
			}
			if (token.type_ == token_array_end)
				break;
		}
		return true;
	}

	bool reader::decode_number(token& token) {
		value decoded;
		if (!decode_number(token, decoded))
			return false;
		current_value().swap_payload(decoded);
		current_value().set_offset_start(token.start_ - begin_);
		current_value().set_offset_limit(token.end_ - begin_);
		return true;
	}

	bool reader::decode_number(token& token, value& decoded) {
		// Attempts to parse the number as an integer. If the number is
		// larger than the maximum supported value of an integer then
		// we decode the number as a double.
		location current = token.start_;
		bool isNegative = *current == '-';
		if (isNegative)
			++current;
		// TODO: Help the compiler do the div and mod at compile time or get rid of them.
		value::largest_uint maxIntegerValue =
			isNegative ? value::largest_uint(value::max_largest_int) + 1
			: value::max_largest_uint;
		value::largest_uint threshold = maxIntegerValue / 10;
		value::largest_uint value = 0;
		while (current < token.end_) {
			char c = *current++;
			if (c < '0' || c > '9')
				return decode_double(token, decoded);
			value::uint digit(c - '0');
			if (value >= threshold) {
				// We've hit or exceeded the max value divided by 10 (rounded down). If
				// a) we've only just touched the limit, b) this is the last digit, and
				// c) it's small enough to fit in that rounding delta, we're okay.
				// Otherwise treat this number as a double to avoid overflow.
				if (value > threshold || current != token.end_ ||
					digit > maxIntegerValue % 10) {
					return decode_double(token, decoded);
				}
			}
			value = value * 10 + digit;
		}
		if (isNegative && value == maxIntegerValue)
			decoded = value::min_largest_int;
		else if (isNegative)
			decoded = -value::largest_int(value);
		else if (value <= value::largest_uint(value::max_int))
			decoded = value::largest_int(value);
		else
			decoded = value;
		return true;
	}

	bool reader::decode_double(token& token) {
		value decoded;
		if (!decode_double(token, decoded))
			return false;
		current_value().swap_payload(decoded);
		current_value().set_offset_start(token.start_ - begin_);
		current_value().set_offset_limit(token.end_ - begin_);
		return true;
	}

	bool reader::decode_double(token& token, value& decoded) {
		double value = 0;
		std::string buffer(token.start_, token.end_);
		std::istringstream is(buffer);
		if (!(is >> value))
			return add_error("'" + std::string(token.start_, token.end_) +
			"' is not a number.",
			token);
		decoded = value;
		return true;
	}

	bool reader::decode_string(token& token) {
		std::string decoded_string;
		if (!decode_string(token, decoded_string))
			return false;
		value decoded(decoded_string);
		current_value().swap_payload(decoded);
		current_value().set_offset_start(token.start_ - begin_);
		current_value().set_offset_limit(token.end_ - begin_);
		return true;
	}

	bool reader::decode_string(token& token, std::string& decoded) {
		decoded.reserve(token.end_ - token.start_ - 2);
		location current = token.start_ + 1; // skip '"'
		location end = token.end_ - 1;       // do not include '"'
		while (current != end) {
			char c = *current++;
			if (c == '"')
				break;
			else if (c == '\\') {
				if (current == end)
					return add_error("Empty escape sequence in string", token, current);
				char escape = *current++;
				switch (escape) {
				case '"':
					decoded += '"';
					break;
				case '/':
					decoded += '/';
					break;
				case '\\':
					decoded += '\\';
					break;
				case 'b':
					decoded += '\b';
					break;
				case 'f':
					decoded += '\f';
					break;
				case 'n':
					decoded += '\n';
					break;
				case 'r':
					decoded += '\r';
					break;
				case 't':
					decoded += '\t';
					break;
				case 'u': {
					unsigned int unicode;
					if (!decode_unicode_code_point(token, current, end, unicode))
						return false;
					decoded += code_point_to_UTF8(unicode);
				} break;
				default:
					return add_error("Bad escape sequence in string", token, current);
				}
			}
			else {
				decoded += c;
			}
		}
		return true;
	}

	bool reader::decode_unicode_code_point(token& token,
		location& current,
		location end,
		unsigned int& unicode) {

		if (!decode_unicode_escape_sequence(token, current, end, unicode))
			return false;
		if (unicode >= 0xD800 && unicode <= 0xDBFF) {
			// surrogate pairs
			if (end - current < 6)
				return add_error(
				"additional six characters expected to parse unicode surrogate pair.",
				token,
				current);
			unsigned int surrogatePair;
			if (*(current++) == '\\' && *(current++) == 'u') {
				if (decode_unicode_escape_sequence(token, current, end, surrogatePair)) {
					unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (surrogatePair & 0x3FF);
				}
				else
					return false;
			}
			else
				return add_error("expecting another \\u token to begin the second half of "
				"a unicode surrogate pair",
				token,
				current);
		}
		return true;
	}

	bool reader::decode_unicode_escape_sequence(token& token,
		location& current,
		location end,
		unsigned int& unicode) {
		if (end - current < 4)
			return add_error(
			"Bad unicode escape sequence in string: four digits expected.",
			token,
			current);
		unicode = 0;
		for (int index = 0; index < 4; ++index) {
			char c = *current++;
			unicode *= 16;
			if (c >= '0' && c <= '9')
				unicode += c - '0';
			else if (c >= 'a' && c <= 'f')
				unicode += c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				unicode += c - 'A' + 10;
			else
				return add_error(
				"Bad unicode escape sequence in string: hexadecimal digit expected.",
				token,
				current);
		}
		return true;
	}

	bool
		reader::add_error(const std::string& message, token& token, location extra) {
		error_info info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = extra;
		errors_.push_back(info);
		return false;
	}

	bool reader::recover_from_error(token_type skipUntilToken) {
		int errorCount = int(errors_.size());
		token skip;
		for (;;) {
			if (!read_token(skip))
				errors_.resize(errorCount); // discard errors caused by recovery
			if (skip.type_ == skipUntilToken || skip.type_ == token_end_of_stream)
				break;
		}
		errors_.resize(errorCount);
		return false;
	}

	bool reader::add_error_and_recover(const std::string& message,
		token& token,
		token_type skipUntilToken) {
		add_error(message, token);
		return recover_from_error(skipUntilToken);
	}

	value& reader::current_value() { return *(nodes_.top()); }

	char reader::get_next_char() {
		if (current_ == end_)
			return 0;
		return *current_++;
	}

	void reader::get_location_line_and_column(location location, int & line, int & column) const
	{
		reader::location current = begin_;
		reader::location lastLineStart = current;
		line = 0;
		while (current < location && current != end_)
		{
			char c = *current++;
			if (c == '\r')
			{
				if (*current == '\n')
					++current;
				lastLineStart = current;
				++line;
			}
			else if (c == '\n')
			{
				lastLineStart = current;
				++line;
			}
		}
		// column & line start at 1
		column = int(location - lastLineStart) + 1;
		++line;
	}

	std::string reader::get_location_line_and_column(location location) const
	{
		int line, column;
		get_location_line_and_column(location, line, column);
		char buffer[18 + 16 + 16 + 1];
		snprintf(buffer, sizeof(buffer), "Line %d, Column %d", line, column);
		return buffer;
	}

	// Deprecated. Preserved for backward compatibility
	std::string reader::get_formated_error_messages() const
	{
		return get_formatted_error_messages();
	}

	std::string reader::get_formatted_error_messages() const
	{
		std::string formattedMessage;
		for (errors::const_iterator itError = errors_.begin();
			itError != errors_.end();
			++itError) {
			const error_info& error = *itError;
			formattedMessage +=
				"* " + get_location_line_and_column(error.token_.start_) + "\n";
			formattedMessage += "  " + error.message_ + "\n";
			if (error.extra_)
				formattedMessage +=
				"See " + get_location_line_and_column(error.extra_) + " for detail.\n";
		}
		return formattedMessage;
	}

	std::vector<reader::structured_error> reader::get_structured_errors() const
	{
		std::vector<reader::structured_error> allErrors;
		for (errors::const_iterator itError = errors_.begin(); itError != errors_.end(); ++itError)
		{
			const error_info& error = *itError;
			reader::structured_error structured;
			structured.offset_start = error.token_.start_ - begin_;
			structured.offset_limit = error.token_.end_ - begin_;
			structured.message = error.message_;
			allErrors.push_back(structured);
		}

		return allErrors;
	}

	bool reader::push_error(const value& value, const std::string& message)
	{
		size_t length = end_ - begin_;
		if (value.get_offset_start() > length || value.get_offset_limit() > length)
			return false;

		token token;
		token.type_ = token_error;
		token.start_ = begin_ + value.get_offset_start();
		token.end_ = end_ + value.get_offset_limit();
		error_info info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = 0;
		errors_.push_back(info);
		return true;
	}

	bool reader::push_error(const value& value, const std::string& message, const json::value& extra)
	{
		size_t length = end_ - begin_;
		if (value.get_offset_start() > length ||
			value.get_offset_limit() > length ||
			extra.get_offset_limit() > length )
				return false;
		token token;
		token.type_ = token_error;
		token.start_ = begin_ + value.get_offset_start();
		token.end_ = begin_ + value.get_offset_limit();
		error_info info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = begin_ + extra.get_offset_start();
		errors_.push_back(info);
		return true;
	}

	bool reader::good() const {
		return !errors_.size();
	}

	// exact copy of features
	class our_features
	{
	public:
		static our_features all();
		bool allow_comments_;
		bool strict_root_;
		bool allow_dropped_null_placeholders_;
		bool allow_numeric_keys_;
		bool allowSingleQuotes_;
		bool failIfExtra_;
		bool rejectDupKeys_;
		bool allowSpecialFloats_;
		int stackLimit_;
	};  // our_features

	// exact copy of Implementation of class features
	// ////////////////////////////////

	our_features our_features::all() { return our_features(); }

	// Implementation of class reader
	// ////////////////////////////////

	// exact copy of reader, renamed to our_reader
	class our_reader
	{
	public:
		typedef const char* location;
		struct structured_error
		{
			size_t offset_start;
			size_t offset_limit;
			std::string message;
		};

		our_reader(our_features const& features);
		bool parse(const char* beginDoc, const char* endDoc, value& root, bool collectComments = true);
		std::string get_formatted_error_messages() const;
		std::vector<structured_error> get_structured_errors() const;
		bool push_error(const value& value, const std::string& message);
		bool push_error(const value& value, const std::string& message, const json::value& extra);
		bool good() const;

	private:
		our_reader(our_reader const&);  // no impl
		void operator=(our_reader const&);  // no impl

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
			token_nan,
			token_pos_inf,
			token_neg_inf,
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

		bool read_token(token& token);
		void skip_spaces();
		bool match(location pattern, int patternLength);
		bool read_comment();
		bool read_cstyle_comment();
		bool read_cpp_style_comment();
		bool read_string();
		bool readStringSingleQuote();
		bool read_number(bool checkInf);
		bool read_value();
		bool read_object(token& token);
		bool read_array(token& token);
		bool decode_number(token& token);
		bool decode_number(token& token, value& decoded);
		bool decode_string(token& token);
		bool decode_string(token& token, std::string& decoded);
		bool decode_double(token& token);
		bool decode_double(token& token, value& decoded);
		bool decode_unicode_code_point(token& token, location& current, location end, unsigned int& unicode);
		bool decode_unicode_escape_sequence(token& token, location& current, location end, unsigned int& unicode);
		bool add_error(const std::string& message, token& token, location extra = 0);
		bool recover_from_error(token_type skipUntilToken);
		bool add_error_and_recover(const std::string& message, token& token, token_type skipUntilToken);
		void skip_until_space();
		value& current_value();
		char get_next_char();
		void get_location_line_and_column(location location, int& line, int& column) const;
		std::string get_location_line_and_column(location location) const;
		void add_comment(location begin, location end, comment_placement placement);
		void skip_comment_tokens(token& token);

		typedef std::stack<value*> Nodes;
		Nodes nodes_;
		errors errors_;
		std::string document_;
		location begin_;
		location end_;
		location current_;
		location lastValueEnd_;
		value* lastValue_;
		std::string commentsBefore_;
		int stackDepth_;

		our_features const features_;
		bool collectComments_;
	};  // our_reader

	// complete copy of Read impl, for our_reader

	our_reader::our_reader(our_features const& features)
		: errors_()
		, document_()
		, begin_()
		, end_()
		, current_()
		, lastValueEnd_()
		, lastValue_()
		, commentsBefore_()
		, stackDepth_(0)
		, features_(features)
		, collectComments_()
	{ }

	bool our_reader::parse(const char* beginDoc, const char* endDoc, value& root, bool collectComments)
	{
		if (!features_.allow_comments_)
			collectComments = false;

		begin_ = beginDoc;
		end_ = endDoc;
		collectComments_ = collectComments;
		current_ = begin_;
		lastValueEnd_ = 0;
		lastValue_ = 0;
		commentsBefore_ = "";
		errors_.clear();
		while (!nodes_.empty())
			nodes_.pop();
		nodes_.push(&root);

		stackDepth_ = 0;
		bool successful = read_value();
		token token;
		skip_comment_tokens(token);
		if (features_.failIfExtra_)
			if (token.type_ != token_error && token.type_ != token_end_of_stream)
			{
				add_error("Extra non-whitespace after JSON value.", token);
				return false;
			}

		if (collectComments_ && !commentsBefore_.empty())
			root.set_comment(commentsBefore_, comment_after);

		if (features_.strict_root_)
			if (!root.is_array() && !root.is_object()) {
				// Set error location to start of doc, ideally should be first token found
				// in doc
				token.type_ = token_error;
				token.start_ = beginDoc;
				token.end_ = endDoc;
				add_error(
					"A valid JSON document must be either an array or an object value.",
					token);
				return false;
			}
		
		return successful;
	}

	bool our_reader::read_value()
	{
		if (stackDepth_ >= features_.stackLimit_) throw_runtime_error("Exceeded stackLimit in read_value().");
		++stackDepth_;
		token token;
		skip_comment_tokens(token);
		bool successful = true;

		if (collectComments_ && !commentsBefore_.empty()) {
			current_value().set_comment(commentsBefore_, comment_before);
			commentsBefore_ = "";
		}

		switch (token.type_) {
		case token_object_begin:
			successful = read_object(token);
			current_value().set_offset_limit(current_ - begin_);
			break;
		case token_array_begin:
			successful = read_array(token);
			current_value().set_offset_limit(current_ - begin_);
			break;
		case token_number:
			successful = decode_number(token);
			break;
		case token_string:
			successful = decode_string(token);
			break;
		case token_true:
		{
			value v(true);
			current_value().swap_payload(v);
			current_value().set_offset_start(token.start_ - begin_);
			current_value().set_offset_limit(token.end_ - begin_);
		}
		break;
		case token_false:
		{
			value v(false);
			current_value().swap_payload(v);
			current_value().set_offset_start(token.start_ - begin_);
			current_value().set_offset_limit(token.end_ - begin_);
		}
		break;
		case token_null:
		{
			value v;
			current_value().swap_payload(v);
			current_value().set_offset_start(token.start_ - begin_);
			current_value().set_offset_limit(token.end_ - begin_);
		}
		break;
		case token_nan:
		{
			value v(std::numeric_limits<double>::quiet_NaN());
			current_value().swap_payload(v);
			current_value().set_offset_start(token.start_ - begin_);
			current_value().set_offset_limit(token.end_ - begin_);
		}
		break;
		case token_pos_inf:
		{
			value v(std::numeric_limits<double>::infinity());
			current_value().swap_payload(v);
			current_value().set_offset_start(token.start_ - begin_);
			current_value().set_offset_limit(token.end_ - begin_);
		}
		break;
		case token_neg_inf:
		{
			value v(-std::numeric_limits<double>::infinity());
			current_value().swap_payload(v);
			current_value().set_offset_start(token.start_ - begin_);
			current_value().set_offset_limit(token.end_ - begin_);
		}
		break;
		case token_array_separator:
		case token_object_end:
		case token_array_end:
			if (features_.allow_dropped_null_placeholders_) {
				// "Un-read" the current token and mark the current value as a null
				// token.
				current_--;
				value v;
				current_value().swap_payload(v);
				current_value().set_offset_start(current_ - begin_ - 1);
				current_value().set_offset_limit(current_ - begin_);
				break;
			} // else, fall through ...
		default:
			current_value().set_offset_start(token.start_ - begin_);
			current_value().set_offset_limit(token.end_ - begin_);
			return add_error("Syntax error: value, object or array expected.", token);
		}

		if (collectComments_) {
			lastValueEnd_ = current_;
			lastValue_ = &current_value();
		}

		--stackDepth_;
		return successful;
	}

	void our_reader::skip_comment_tokens(token& token) {
		if (features_.allow_comments_) {
			do {
				read_token(token);
			} while (token.type_ == token_comment);
		}
		else {
			read_token(token);
		}
	}

	bool our_reader::read_token(token& token) {
		skip_spaces();
		token.start_ = current_;
		char c = get_next_char();
		bool ok = true;
		switch (c) {
		case '{':
			token.type_ = token_object_begin;
			break;
		case '}':
			token.type_ = token_object_end;
			break;
		case '[':
			token.type_ = token_array_begin;
			break;
		case ']':
			token.type_ = token_array_end;
			break;
		case '"':
			token.type_ = token_string;
			ok = read_string();
			break;
		case '\'':
			if (features_.allowSingleQuotes_) {
				token.type_ = token_string;
				ok = readStringSingleQuote();
				break;
			} // else continue
		case '/':
			token.type_ = token_comment;
			ok = read_comment();
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			token.type_ = token_number;
			read_number(false);
			break;
		case '-':
			if (read_number(true)) {
				token.type_ = token_number;
			}
			else {
				token.type_ = token_neg_inf;
				ok = features_.allowSpecialFloats_ && match("nfinity", 7);
			}
			break;
		case 't':
			token.type_ = token_true;
			ok = match("rue", 3);
			break;
		case 'f':
			token.type_ = token_false;
			ok = match("alse", 4);
			break;
		case 'n':
			token.type_ = token_null;
			ok = match("ull", 3);
			break;
		case 'N':
			if (features_.allowSpecialFloats_) {
				token.type_ = token_nan;
				ok = match("aN", 2);
			}
			else {
				ok = false;
			}
			break;
		case 'I':
			if (features_.allowSpecialFloats_) {
				token.type_ = token_pos_inf;
				ok = match("nfinity", 7);
			}
			else {
				ok = false;
			}
			break;
		case ',':
			token.type_ = token_array_separator;
			break;
		case ':':
			token.type_ = token_member_separator;
			break;
		case 0:
			token.type_ = token_end_of_stream;
			break;
		default:
			ok = false;
			break;
		}
		if (!ok)
			token.type_ = token_error;
		token.end_ = current_;
		return true;
	}

	void our_reader::skip_spaces() {
		while (current_ != end_) {
			char c = *current_;
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
				++current_;
			else
				break;
		}
	}

	bool our_reader::match(location pattern, int patternLength) {
		if (end_ - current_ < patternLength)
			return false;
		int index = patternLength;
		while (index--)
			if (current_[index] != pattern[index])
				return false;
		current_ += patternLength;
		return true;
	}

	bool our_reader::read_comment() {
		location commentBegin = current_ - 1;
		char c = get_next_char();
		bool successful = false;
		if (c == '*')
			successful = read_cstyle_comment();
		else if (c == '/')
			successful = read_cpp_style_comment();
		if (!successful)
			return false;

		if (collectComments_) {
			comment_placement placement = comment_before;
			if (lastValueEnd_ && !containsNewLine(lastValueEnd_, commentBegin)) {
				if (c != '*' || !containsNewLine(commentBegin, current_))
					placement = comment_after_on_same_line;
			}

			add_comment(commentBegin, current_, placement);
		}
		return true;
	}

	void
		our_reader::add_comment(location begin, location end, comment_placement placement) {
		assert(collectComments_);
		const std::string& normalized = normalizeEOL(begin, end);
		if (placement == comment_after_on_same_line) {
			assert(lastValue_ != 0);
			lastValue_->set_comment(normalized, placement);
		}
		else {
			commentsBefore_ += normalized;
		}
	}

	bool our_reader::read_cstyle_comment() {
		while (current_ != end_) {
			char c = get_next_char();
			if (c == '*' && *current_ == '/')
				break;
		}
		return get_next_char() == '/';
	}

	bool our_reader::read_cpp_style_comment() {
		while (current_ != end_) {
			char c = get_next_char();
			if (c == '\n')
				break;
			if (c == '\r') {
				// Consume DOS EOL. It will be normalized in add_comment.
				if (current_ != end_ && *current_ == '\n')
					get_next_char();
				// Break on Moc OS 9 EOL.
				break;
			}
		}
		return true;
	}

	bool our_reader::read_number(bool checkInf) {
		const char *p = current_;
		if (checkInf && p != end_ && *p == 'I') {
			current_ = ++p;
			return false;
		}
		char c = '0'; // stopgap for already consumed character
		// integral part
		while (c >= '0' && c <= '9')
			c = (current_ = p) < end_ ? *p++ : 0;
		// fractional part
		if (c == '.') {
			c = (current_ = p) < end_ ? *p++ : 0;
			while (c >= '0' && c <= '9')
				c = (current_ = p) < end_ ? *p++ : 0;
		}
		// exponential part
		if (c == 'e' || c == 'E') {
			c = (current_ = p) < end_ ? *p++ : 0;
			if (c == '+' || c == '-')
				c = (current_ = p) < end_ ? *p++ : 0;
			while (c >= '0' && c <= '9')
				c = (current_ = p) < end_ ? *p++ : 0;
		}
		return true;
	}
	bool our_reader::read_string() {
		char c = 0;
		while (current_ != end_) {
			c = get_next_char();
			if (c == '\\')
				get_next_char();
			else if (c == '"')
				break;
		}
		return c == '"';
	}


	bool our_reader::readStringSingleQuote() {
		char c = 0;
		while (current_ != end_) {
			c = get_next_char();
			if (c == '\\')
				get_next_char();
			else if (c == '\'')
				break;
		}
		return c == '\'';
	}

	bool our_reader::read_object(token& tokenStart) {
		token tokenName;
		std::string name;
		value init(object_value);
		current_value().swap_payload(init);
		current_value().set_offset_start(tokenStart.start_ - begin_);
		while (read_token(tokenName)) {
			bool initialTokenOk = true;
			while (tokenName.type_ == token_comment && initialTokenOk)
				initialTokenOk = read_token(tokenName);
			if (!initialTokenOk)
				break;
			if (tokenName.type_ == token_object_end && name.empty()) // empty object
				return true;
			name = "";
			if (tokenName.type_ == token_string) {
				if (!decode_string(tokenName, name))
					return recover_from_error(token_object_end);
			}
			else if (tokenName.type_ == token_number && features_.allow_numeric_keys_) {
				value numberName;
				if (!decode_number(tokenName, numberName))
					return recover_from_error(token_object_end);
				name = numberName.as_string();
			}
			else {
				break;
			}

			token colon;
			if (!read_token(colon) || colon.type_ != token_member_separator) {
				return add_error_and_recover(
					"Missing ':' after object member name", colon, token_object_end);
			}
			if (name.length() >= (1U << 30)) throw_runtime_error("keylength >= 2^30");
			if (features_.rejectDupKeys_ && current_value().is_member(name)) {
				std::string msg = "Duplicate key: '" + name + "'";
				return add_error_and_recover(
					msg, tokenName, token_object_end);
			}
			value& value = current_value()[name];
			nodes_.push(&value);
			bool ok = read_value();
			nodes_.pop();
			if (!ok) // error already set
				return recover_from_error(token_object_end);

			token comma;
			if (!read_token(comma) ||
				(comma.type_ != token_object_end && comma.type_ != token_array_separator &&
				comma.type_ != token_comment)) {
				return add_error_and_recover(
					"Missing ',' or '}' in object declaration", comma, token_object_end);
			}
			bool finalizeTokenOk = true;
			while (comma.type_ == token_comment && finalizeTokenOk)
				finalizeTokenOk = read_token(comma);
			if (comma.type_ == token_object_end)
				return true;
		}
		return add_error_and_recover(
			"Missing '}' or object member name", tokenName, token_object_end);
	}

	bool our_reader::read_array(token& tokenStart) {
		value init(array_value);
		current_value().swap_payload(init);
		current_value().set_offset_start(tokenStart.start_ - begin_);
		skip_spaces();
		if (*current_ == ']') // empty array
		{
			token endArray;
			read_token(endArray);
			return true;
		}
		int index = 0;
		for (;;) {
			value& value = current_value()[index++];
			nodes_.push(&value);
			bool ok = read_value();
			nodes_.pop();
			if (!ok) // error already set
				return recover_from_error(token_array_end);

			token token;
			// Accept Comment after last item in the array.
			ok = read_token(token);
			while (token.type_ == token_comment && ok) {
				ok = read_token(token);
			}
			bool badTokenType =
				(token.type_ != token_array_separator && token.type_ != token_array_end);
			if (!ok || badTokenType) {
				return add_error_and_recover(
					"Missing ',' or ']' in array declaration", token, token_array_end);
			}
			if (token.type_ == token_array_end)
				break;
		}
		return true;
	}

	bool our_reader::decode_number(token& token) {
		value decoded;
		if (!decode_number(token, decoded))
			return false;
		current_value().swap_payload(decoded);
		current_value().set_offset_start(token.start_ - begin_);
		current_value().set_offset_limit(token.end_ - begin_);
		return true;
	}

	bool our_reader::decode_number(token& token, value& decoded) {
		// Attempts to parse the number as an integer. If the number is
		// larger than the maximum supported value of an integer then
		// we decode the number as a double.
		location current = token.start_;
		bool isNegative = *current == '-';
		if (isNegative)
			++current;
		// TODO: Help the compiler do the div and mod at compile time or get rid of them.
		value::largest_uint maxIntegerValue =
			isNegative ? value::largest_uint(-value::min_largest_int)
			: value::max_largest_uint;
		value::largest_uint threshold = maxIntegerValue / 10;
		value::largest_uint value = 0;
		while (current < token.end_) {
			char c = *current++;
			if (c < '0' || c > '9')
				return decode_double(token, decoded);
			value::uint digit(c - '0');
			if (value >= threshold) {
				// We've hit or exceeded the max value divided by 10 (rounded down). If
				// a) we've only just touched the limit, b) this is the last digit, and
				// c) it's small enough to fit in that rounding delta, we're okay.
				// Otherwise treat this number as a double to avoid overflow.
				if (value > threshold || current != token.end_ ||
					digit > maxIntegerValue % 10) {
					return decode_double(token, decoded);
				}
			}
			value = value * 10 + digit;
		}
		if (isNegative)
			decoded = -value::largest_int(value);
		else if (value <= value::largest_uint(value::max_int))
			decoded = value::largest_int(value);
		else
			decoded = value;
		return true;
	}

	bool our_reader::decode_double(token& token) {
		value decoded;
		if (!decode_double(token, decoded))
			return false;
		current_value().swap_payload(decoded);
		current_value().set_offset_start(token.start_ - begin_);
		current_value().set_offset_limit(token.end_ - begin_);
		return true;
	}

	bool our_reader::decode_double(token& token, value& decoded) {
		double value = 0;
		const int bufferSize = 32;
		int count;
		int length = int(token.end_ - token.start_);

		// Sanity check to avoid buffer overflow exploits.
		if (length < 0) {
			return add_error("Unable to parse token length", token);
		}

		// Avoid using a string constant for the format control string given to
		// sscanf, as this can cause hard to debug crashes on OS X. See here for more
		// info:
		//
		//     http://developer.apple.com/library/mac/#DOCUMENTATION/DeveloperTools/gcc-4.0.1/gcc/Incompatibilities.html
		char format[] = "%lf";

		if (length <= bufferSize) {
			char buffer[bufferSize + 1];
			memcpy(buffer, token.start_, length);
			buffer[length] = 0;
			count = sscanf(buffer, format, &value);
		}
		else {
			std::string buffer(token.start_, token.end_);
			count = sscanf(buffer.c_str(), format, &value);
		}

		if (count != 1)
			return add_error("'" + std::string(token.start_, token.end_) +
			"' is not a number.",
			token);
		decoded = value;
		return true;
	}

	bool our_reader::decode_string(token& token) {
		std::string decoded_string;
		if (!decode_string(token, decoded_string))
			return false;
		value decoded(decoded_string);
		current_value().swap_payload(decoded);
		current_value().set_offset_start(token.start_ - begin_);
		current_value().set_offset_limit(token.end_ - begin_);
		return true;
	}

	bool our_reader::decode_string(token& token, std::string& decoded) {
		decoded.reserve(token.end_ - token.start_ - 2);
		location current = token.start_ + 1; // skip '"'
		location end = token.end_ - 1;       // do not include '"'
		while (current != end) {
			char c = *current++;
			if (c == '"')
				break;
			else if (c == '\\') {
				if (current == end)
					return add_error("Empty escape sequence in string", token, current);
				char escape = *current++;
				switch (escape) {
				case '"':
					decoded += '"';
					break;
				case '/':
					decoded += '/';
					break;
				case '\\':
					decoded += '\\';
					break;
				case 'b':
					decoded += '\b';
					break;
				case 'f':
					decoded += '\f';
					break;
				case 'n':
					decoded += '\n';
					break;
				case 'r':
					decoded += '\r';
					break;
				case 't':
					decoded += '\t';
					break;
				case 'u': {
					unsigned int unicode;
					if (!decode_unicode_code_point(token, current, end, unicode))
						return false;
					decoded += code_point_to_UTF8(unicode);
				} break;
				default:
					return add_error("Bad escape sequence in string", token, current);
				}
			}
			else {
				decoded += c;
			}
		}
		return true;
	}

	bool our_reader::decode_unicode_code_point(token& token,
		location& current,
		location end,
		unsigned int& unicode) {

		if (!decode_unicode_escape_sequence(token, current, end, unicode))
			return false;
		if (unicode >= 0xD800 && unicode <= 0xDBFF) {
			// surrogate pairs
			if (end - current < 6)
				return add_error(
				"additional six characters expected to parse unicode surrogate pair.",
				token,
				current);
			unsigned int surrogatePair;
			if (*(current++) == '\\' && *(current++) == 'u') {
				if (decode_unicode_escape_sequence(token, current, end, surrogatePair)) {
					unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (surrogatePair & 0x3FF);
				}
				else
					return false;
			}
			else
				return add_error("expecting another \\u token to begin the second half of "
				"a unicode surrogate pair",
				token,
				current);
		}
		return true;
	}

	bool our_reader::decode_unicode_escape_sequence(token& token,
		location& current,
		location end,
		unsigned int& unicode) {
		if (end - current < 4)
			return add_error(
			"Bad unicode escape sequence in string: four digits expected.",
			token,
			current);
		unicode = 0;
		for (int index = 0; index < 4; ++index) {
			char c = *current++;
			unicode *= 16;
			if (c >= '0' && c <= '9')
				unicode += c - '0';
			else if (c >= 'a' && c <= 'f')
				unicode += c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				unicode += c - 'A' + 10;
			else
				return add_error(
				"Bad unicode escape sequence in string: hexadecimal digit expected.",
				token,
				current);
		}
		return true;
	}

	bool
		our_reader::add_error(const std::string& message, token& token, location extra) {
		error_info info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = extra;
		errors_.push_back(info);
		return false;
	}

	bool our_reader::recover_from_error(token_type skipUntilToken) {
		int errorCount = int(errors_.size());
		token skip;
		for (;;) {
			if (!read_token(skip))
				errors_.resize(errorCount); // discard errors caused by recovery
			if (skip.type_ == skipUntilToken || skip.type_ == token_end_of_stream)
				break;
		}
		errors_.resize(errorCount);
		return false;
	}

	bool our_reader::add_error_and_recover(const std::string& message,
		token& token,
		token_type skipUntilToken) {
		add_error(message, token);
		return recover_from_error(skipUntilToken);
	}

	value& our_reader::current_value() { return *(nodes_.top()); }

	char our_reader::get_next_char() {
		if (current_ == end_)
			return 0;
		return *current_++;
	}

	void our_reader::get_location_line_and_column(location location,
		int& line,
		int& column) const {
		our_reader::location current = begin_;
		our_reader::location lastLineStart = current;
		line = 0;
		while (current < location && current != end_) {
			char c = *current++;
			if (c == '\r') {
				if (*current == '\n')
					++current;
				lastLineStart = current;
				++line;
			}
			else if (c == '\n') {
				lastLineStart = current;
				++line;
			}
		}
		// column & line start at 1
		column = int(location - lastLineStart) + 1;
		++line;
	}

	std::string our_reader::get_location_line_and_column(location location) const {
		int line, column;
		get_location_line_and_column(location, line, column);
		char buffer[18 + 16 + 16 + 1];
		snprintf(buffer, sizeof(buffer), "Line %d, Column %d", line, column);
		return buffer;
	}

	std::string our_reader::get_formatted_error_messages() const {
		std::string formattedMessage;
		for (errors::const_iterator itError = errors_.begin();
			itError != errors_.end();
			++itError) {
			const error_info& error = *itError;
			formattedMessage +=
				"* " + get_location_line_and_column(error.token_.start_) + "\n";
			formattedMessage += "  " + error.message_ + "\n";
			if (error.extra_)
				formattedMessage +=
				"See " + get_location_line_and_column(error.extra_) + " for detail.\n";
		}
		return formattedMessage;
	}

	std::vector<our_reader::structured_error> our_reader::get_structured_errors() const {
		std::vector<our_reader::structured_error> allErrors;
		for (errors::const_iterator itError = errors_.begin();
			itError != errors_.end();
			++itError) {
			const error_info& error = *itError;
			our_reader::structured_error structured;
			structured.offset_start = error.token_.start_ - begin_;
			structured.offset_limit = error.token_.end_ - begin_;
			structured.message = error.message_;
			allErrors.push_back(structured);
		}
		return allErrors;
	}

	bool our_reader::push_error(const value& value, const std::string& message) {
		size_t length = end_ - begin_;
		if (value.get_offset_start() > length
			|| value.get_offset_limit() > length)
			return false;
		token token;
		token.type_ = token_error;
		token.start_ = begin_ + value.get_offset_start();
		token.end_ = end_ + value.get_offset_limit();
		error_info info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = 0;
		errors_.push_back(info);
		return true;
	}

	bool our_reader::push_error(const value& value, const std::string& message, const json::value& extra) {
		size_t length = end_ - begin_;
		if (value.get_offset_start() > length
			|| value.get_offset_limit() > length
			|| extra.get_offset_limit() > length)
			return false;
		token token;
		token.type_ = token_error;
		token.start_ = begin_ + value.get_offset_start();
		token.end_ = begin_ + value.get_offset_limit();
		error_info info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = begin_ + extra.get_offset_start();
		errors_.push_back(info);
		return true;
	}

	bool our_reader::good() const {
		return !errors_.size();
	}


	class OurCharReader : public char_reader {
		bool const collectComments_;
		our_reader reader_;
	public:
		OurCharReader(
			bool collectComments,
			our_features const& features)
			: collectComments_(collectComments)
			, reader_(features)
		{}
		bool parse(
			char const* beginDoc, char const* endDoc,
			value* root, std::string* errs) override {
			bool ok = reader_.parse(beginDoc, endDoc, *root, collectComments_);
			if (errs) {
				*errs = reader_.get_formatted_error_messages();
			}
			return ok;
		}
	};

	char_reader_builder::char_reader_builder()
	{
		set_defaults(&settings_);
	}
	char_reader_builder::~char_reader_builder()
	{}
	char_reader* char_reader_builder::new_char_reader() const
	{
		bool collectComments = settings_["collectComments"].as_bool();
		our_features features = our_features::all();
		features.allow_comments_ = settings_["allowComments"].as_bool();
		features.strict_root_ = settings_["strictRoot"].as_bool();
		features.allow_dropped_null_placeholders_ = settings_["allowDroppedNullPlaceholders"].as_bool();
		features.allow_numeric_keys_ = settings_["allowNumericKeys"].as_bool();
		features.allowSingleQuotes_ = settings_["allowSingleQuotes"].as_bool();
		features.stackLimit_ = settings_["stackLimit"].as_int();
		features.failIfExtra_ = settings_["failIfExtra"].as_bool();
		features.rejectDupKeys_ = settings_["rejectDupKeys"].as_bool();
		features.allowSpecialFloats_ = settings_["allowSpecialFloats"].as_bool();
		return new OurCharReader(collectComments, features);
	}
	static void getValidReaderKeys(std::set<std::string>* valid_keys)
	{
		valid_keys->clear();
		valid_keys->insert("collectComments");
		valid_keys->insert("allowComments");
		valid_keys->insert("strictRoot");
		valid_keys->insert("allowDroppedNullPlaceholders");
		valid_keys->insert("allowNumericKeys");
		valid_keys->insert("allowSingleQuotes");
		valid_keys->insert("stackLimit");
		valid_keys->insert("failIfExtra");
		valid_keys->insert("rejectDupKeys");
		valid_keys->insert("allowSpecialFloats");
	}
	bool char_reader_builder::validate(json::value* invalid) const
	{
		json::value my_invalid;
		if (!invalid) invalid = &my_invalid;  // so we do not need to test for NULL
		json::value& inv = *invalid;
		std::set<std::string> valid_keys;
		getValidReaderKeys(&valid_keys);
		value::Members keys = settings_.get_member_names();
		size_t n = keys.size();
		for (size_t i = 0; i < n; ++i) {
			std::string const& key = keys[i];
			if (valid_keys.find(key) == valid_keys.end()) {
				inv[key] = settings_[key];
			}
		}
		return 0u == inv.size();
	}
	value& char_reader_builder::operator[](std::string key)
	{
		return settings_[key];
	}
	// static
	void char_reader_builder::strict_mode(json::value* settings)
	{
		//! [CharReaderBuilderStrictMode]
		(*settings)["allowComments"] = false;
		(*settings)["strictRoot"] = true;
		(*settings)["allowDroppedNullPlaceholders"] = false;
		(*settings)["allowNumericKeys"] = false;
		(*settings)["allowSingleQuotes"] = false;
		(*settings)["stackLimit"] = 1000;
		(*settings)["failIfExtra"] = true;
		(*settings)["rejectDupKeys"] = true;
		(*settings)["allowSpecialFloats"] = false;
		//! [CharReaderBuilderStrictMode]
	}
	// static
	void char_reader_builder::set_defaults(json::value* settings)
	{
		//! [CharReaderBuilderDefaults]
		(*settings)["collectComments"] = true;
		(*settings)["allowComments"] = true;
		(*settings)["strictRoot"] = false;
		(*settings)["allowDroppedNullPlaceholders"] = false;
		(*settings)["allowNumericKeys"] = false;
		(*settings)["allowSingleQuotes"] = false;
		(*settings)["stackLimit"] = 1000;
		(*settings)["failIfExtra"] = false;
		(*settings)["rejectDupKeys"] = false;
		(*settings)["allowSpecialFloats"] = false;
		//! [CharReaderBuilderDefaults]
	}

	//////////////////////////////////
	// global functions

	bool parseFromStream(
		char_reader::factory const& fact, std::istream& sin,
		value* root, std::string* errs)
	{
		std::ostringstream ssin;
		ssin << sin.rdbuf();
		std::string doc = ssin.str();
		char const* begin = doc.data();
		char const* end = begin + doc.size();
		// Note that we do not actually need a null-terminator.
		char_reader_ptr const reader(fact.new_char_reader());
		return reader->parse(begin, end, root, errs);
	}

	std::istream& operator>>(std::istream& sin, value& root) {
		char_reader_builder b;
		std::string errs;
		bool ok = parseFromStream(b, sin, &root, &errs);
		if (!ok) {
			fprintf(stderr,
				"Error from reader: %s",
				errs.c_str());

			throw_runtime_error("reader error");
		}
		return sin;
	}

} // namespace json
