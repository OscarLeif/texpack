// Copyright 2011 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
#include <json/writer.h>
#include "json_tool.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <iomanip>
#include <memory>
#include <sstream>
#include <utility>
#include <set>
#include <cassert>
#include <cstring>
#include <cstdio>

#if defined(_MSC_VER) && _MSC_VER >= 1200 && _MSC_VER < 1800 // Between VC++ 6.0 and VC++ 11.0
#include <float.h>
#define isfinite _finite
#elif defined(__sun) && defined(__SVR4) //Solaris
#if !defined(isfinite)
#include <ieeefp.h>
#define isfinite finite
#endif
#elif defined(_AIX)
#if !defined(isfinite)
#include <math.h>
#define isfinite finite
#endif
#elif defined(__hpux)
#if !defined(isfinite)
#if defined(__ia64) && !defined(finite)
#define isfinite(x) ((sizeof(x) == sizeof(float) ? \
                     _Isfinitef(x) : _IsFinite(x)))
#else
#include <math.h>
#define isfinite finite
#endif
#endif
#else
#include <cmath>
#if !(defined(__QNXNTO__)) // QNX already defines isfinite
#define isfinite std::isfinite
#endif
#endif

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

#if defined(__BORLANDC__)  
#include <float.h>
#define isfinite _finite
#define snprintf _snprintf
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400 // VC++ 8.0
// Disable warning about strdup being deprecated.
#pragma warning(disable : 4996)
#endif

namespace json {

#if __cplusplus >= 201103L || (defined(_CPPLIB_VER) && _CPPLIB_VER >= 520)
	typedef std::unique_ptr<stream_writer> StreamWriterPtr;
#else
	typedef std::auto_ptr<stream_writer>   StreamWriterPtr;
#endif

	static bool containsControlCharacter(const char* str) {
		while (*str) {
			if (is_control_character(*(str++)))
				return true;
		}
		return false;
	}

	static bool containsControlCharacter0(const char* str, unsigned len) {
		char const* end = str + len;
		while (end != str) {
			if (is_control_character(*str) || 0 == *str)
				return true;
			++str;
		}
		return false;
	}

	std::string valueToString(largest_int value) {
		uint_to_string_buffer buffer;
		char* current = buffer + sizeof(buffer);
		if (value == value::min_largest_int) {
			uint_to_string(largest_uint(value::max_largest_int) + 1, current);
			*--current = '-';
		}
		else if (value < 0) {
			uint_to_string(largest_uint(-value), current);
			*--current = '-';
		}
		else {
			uint_to_string(largest_uint(value), current);
		}
		assert(current >= buffer);
		return current;
	}

	std::string valueToString(largest_uint value) {
		uint_to_string_buffer buffer;
		char* current = buffer + sizeof(buffer);
		uint_to_string(value, current);
		assert(current >= buffer);
		return current;
	}

#if defined(JSON_HAS_INT64)

	std::string valueToString(int value) {
		return valueToString(largest_int(value));
	}

	std::string valueToString(uint value) {
		return valueToString(largest_uint(value));
	}

#endif // # if defined(JSON_HAS_INT64)

	std::string valueToString(double value, bool useSpecialFloats) {
		// Allocate a buffer that is more than large enough to store the 16 digits of
		// precision requested below.
		char buffer[32];
		int len = -1;

		// Print into the buffer. We need not request the alternative representation
		// that always has a decimal point because JSON doesn't distingish the
		// concepts of reals and integers.
		if (isfinite(value)) {
			len = snprintf(buffer, sizeof(buffer), "%.17g", value);
		}
		else {
			// IEEE standard states that NaN values will not compare to themselves
			if (value != value) {
				len = snprintf(buffer, sizeof(buffer), useSpecialFloats ? "NaN" : "null");
			}
			else if (value < 0) {
				len = snprintf(buffer, sizeof(buffer), useSpecialFloats ? "-Infinity" : "-1e+9999");
			}
			else {
				len = snprintf(buffer, sizeof(buffer), useSpecialFloats ? "Infinity" : "1e+9999");
			}
			// For those, we do not need to call fixNumLoc, but it is fast.
		}
		assert(len >= 0);
		fix_numeric_locale(buffer, buffer + len);
		return buffer;
	}

	std::string valueToString(double value) { return valueToString(value, false); }

	std::string valueToString(bool value) { return value ? "true" : "false"; }

	std::string valueToQuotedString(const char* value) {
		if (value == NULL)
			return "";
		// Not sure how to handle unicode...
		if (strpbrk(value, "\"\\\b\f\n\r\t") == NULL &&
			!containsControlCharacter(value))
			return std::string("\"") + value + "\"";
		// We have to walk value and escape any special characters.
		// Appending to std::string is not efficient, but this should be rare.
		// (Note: forward slashes are *not* rare, but I am not escaping them.)
		std::string::size_type maxsize =
			strlen(value) * 2 + 3; // allescaped+quotes+NULL
		std::string result;
		result.reserve(maxsize); // to avoid lots of mallocs
		result += "\"";
		for (const char* c = value; *c != 0; ++c) {
			switch (*c) {
			case '\"':
				result += "\\\"";
				break;
			case '\\':
				result += "\\\\";
				break;
			case '\b':
				result += "\\b";
				break;
			case '\f':
				result += "\\f";
				break;
			case '\n':
				result += "\\n";
				break;
			case '\r':
				result += "\\r";
				break;
			case '\t':
				result += "\\t";
				break;
				// case '/':
				// Even though \/ is considered a legal escape in JSON, a bare
				// slash is also legal, so I see no reason to escape it.
				// (I hope I am not misunderstanding something.
				// blep notes: actually escaping \/ may be useful in javascript to avoid </
				// sequence.
				// Should add a flag to allow this compatibility mode and prevent this
				// sequence from occurring.
			default:
				if (is_control_character(*c)) {
					std::ostringstream oss;
					oss << "\\u" << std::hex << std::uppercase << std::setfill('0')
						<< std::setw(4) << static_cast<int>(*c);
					result += oss.str();
				}
				else {
					result += *c;
				}
				break;
			}
		}
		result += "\"";
		return result;
	}

	// https://github.com/upcaste/upcaste/blob/master/src/upcore/src/cstring/strnpbrk.cpp
	static char const* strnpbrk(char const* s, char const* accept, size_t n) {
		assert((s || !n) && accept);

		char const* const end = s + n;
		for (char const* cur = s; cur < end; ++cur) {
			int const c = *cur;
			for (char const* a = accept; *a; ++a) {
				if (*a == c) {
					return cur;
				}
			}
		}
		return NULL;
	}
	static std::string valueToQuotedStringN(const char* value, unsigned length) {
		if (value == NULL)
			return "";
		// Not sure how to handle unicode...
		if (strnpbrk(value, "\"\\\b\f\n\r\t", length) == NULL &&
			!containsControlCharacter0(value, length))
			return std::string("\"") + value + "\"";
		// We have to walk value and escape any special characters.
		// Appending to std::string is not efficient, but this should be rare.
		// (Note: forward slashes are *not* rare, but I am not escaping them.)
		std::string::size_type maxsize =
			length * 2 + 3; // allescaped+quotes+NULL
		std::string result;
		result.reserve(maxsize); // to avoid lots of mallocs
		result += "\"";
		char const* end = value + length;
		for (const char* c = value; c != end; ++c) {
			switch (*c) {
			case '\"':
				result += "\\\"";
				break;
			case '\\':
				result += "\\\\";
				break;
			case '\b':
				result += "\\b";
				break;
			case '\f':
				result += "\\f";
				break;
			case '\n':
				result += "\\n";
				break;
			case '\r':
				result += "\\r";
				break;
			case '\t':
				result += "\\t";
				break;
				// case '/':
				// Even though \/ is considered a legal escape in JSON, a bare
				// slash is also legal, so I see no reason to escape it.
				// (I hope I am not misunderstanding something.)
				// blep notes: actually escaping \/ may be useful in javascript to avoid </
				// sequence.
				// Should add a flag to allow this compatibility mode and prevent this
				// sequence from occurring.
			default:
				if ((is_control_character(*c)) || (*c == 0)) {
					std::ostringstream oss;
					oss << "\\u" << std::hex << std::uppercase << std::setfill('0')
						<< std::setw(4) << static_cast<int>(*c);
					result += oss.str();
				}
				else {
					result += *c;
				}
				break;
			}
		}
		result += "\"";
		return result;
	}

	// Class writer
	// //////////////////////////////////////////////////////////////////
	writer::~writer() {}

	// Class fast_writer
	// //////////////////////////////////////////////////////////////////

	fast_writer::fast_writer()
		: yamlCompatiblityEnabled_(false), dropNullPlaceholders_(false),
		omitEndingLineFeed_(false) {}

	void fast_writer::enable_YAML_compatibility() { yamlCompatiblityEnabled_ = true; }

	void fast_writer::dropNullPlaceholders() { dropNullPlaceholders_ = true; }

	void fast_writer::omitEndingLineFeed() { omitEndingLineFeed_ = true; }

	std::string fast_writer::write(const value& root) {
		document_ = "";
		write_value(root);
		if (!omitEndingLineFeed_)
			document_ += "\n";
		return document_;
	}

	void fast_writer::write_value(const value& value) {
		switch (value.type()) {
		case null_value:
			if (!dropNullPlaceholders_)
				document_ += "null";
			break;
		case int_value:
			document_ += valueToString(value.as_largest_int());
			break;
		case uint_value:
			document_ += valueToString(value.as_largest_uint());
			break;
		case real_value:
			document_ += valueToString(value.as_double());
			break;
		case string_value:
		{
			// Is NULL possible for value.string_?
			char const* str;
			char const* end;
			bool ok = value.get_string(&str, &end);
			if (ok) document_ += valueToQuotedStringN(str, static_cast<unsigned>(end - str));
			break;
		}
		case boolean_value:
			document_ += valueToString(value.as_bool());
			break;
		case array_value: {
			document_ += '[';
			int size = value.size();
			for (int index = 0; index < size; ++index) {
				if (index > 0)
					document_ += ',';
				write_value(value[index]);
			}
			document_ += ']';
		} break;
		case object_value: {
			value::Members members(value.get_member_names());
			document_ += '{';
			for (value::Members::iterator it = members.begin(); it != members.end();
				++it) {
				const std::string& name = *it;
				if (it != members.begin())
					document_ += ',';
				document_ += valueToQuotedStringN(name.data(), static_cast<unsigned>(name.length()));
				document_ += yamlCompatiblityEnabled_ ? ": " : ":";
				write_value(value[name]);
			}
			document_ += '}';
		} break;
		}
	}

	// Class styled_writer
	// //////////////////////////////////////////////////////////////////

	styled_writer::styled_writer()
		: rightMargin_(74), indentSize_(3), addChildValues_() {}

	std::string styled_writer::write(const value& root) {
		document_ = "";
		addChildValues_ = false;
		indentString_ = "";
		write_comment_before_value(root);
		write_value(root);
		write_comment_after_value_on_same_line(root);
		document_ += "\n";
		return document_;
	}

	void styled_writer::write_value(const value& value) {
		switch (value.type()) {
		case null_value:
			push_value("null");
			break;
		case int_value:
			push_value(valueToString(value.as_largest_int()));
			break;
		case uint_value:
			push_value(valueToString(value.as_largest_uint()));
			break;
		case real_value:
			push_value(valueToString(value.as_double()));
			break;
		case string_value:
		{
			// Is NULL possible for value.string_?
			char const* str;
			char const* end;
			bool ok = value.get_string(&str, &end);
			if (ok) push_value(valueToQuotedStringN(str, static_cast<unsigned>(end - str)));
			else push_value("");
			break;
		}
		case boolean_value:
			push_value(valueToString(value.as_bool()));
			break;
		case array_value:
			write_array_value(value);
			break;
		case object_value: {
			value::Members members(value.get_member_names());
			if (members.empty())
				push_value("{}");
			else {
				write_with_indent("{");
				indent();
				value::Members::iterator it = members.begin();
				for (;;) {
					const std::string& name = *it;
					const json::value& childValue = value[name];
					write_comment_before_value(childValue);
					write_with_indent(valueToQuotedString(name.c_str()));
					document_ += " : ";
					write_value(childValue);
					if (++it == members.end()) {
						write_comment_after_value_on_same_line(childValue);
						break;
					}
					document_ += ',';
					write_comment_after_value_on_same_line(childValue);
				}
				unindent();
				write_with_indent("}");
			}
		} break;
		}
	}

	void styled_writer::write_array_value(const value& value) {
		unsigned size = value.size();
		if (size == 0)
			push_value("[]");
		else {
			bool isArrayMultiLine = is_multine_array(value);
			if (isArrayMultiLine) {
				write_with_indent("[");
				indent();
				bool hasChildValue = !childValues_.empty();
				unsigned index = 0;
				for (;;) {
					const json::value& childValue = value[index];
					write_comment_before_value(childValue);
					if (hasChildValue)
						write_with_indent(childValues_[index]);
					else {
						write_indent();
						write_value(childValue);
					}
					if (++index == size) {
						write_comment_after_value_on_same_line(childValue);
						break;
					}
					document_ += ',';
					write_comment_after_value_on_same_line(childValue);
				}
				unindent();
				write_with_indent("]");
			}
			else // output on a single line
			{
				assert(childValues_.size() == size);
				document_ += "[ ";
				for (unsigned index = 0; index < size; ++index) {
					if (index > 0)
						document_ += ", ";
					document_ += childValues_[index];
				}
				document_ += " ]";
			}
		}
	}

	bool styled_writer::is_multine_array(const value& value) {
		int size = value.size();
		bool isMultiLine = size * 3 >= rightMargin_;
		childValues_.clear();
		for (int index = 0; index < size && !isMultiLine; ++index) {
			const json::value& childValue = value[index];
			isMultiLine = ((childValue.is_array() || childValue.is_object()) &&
				childValue.size() > 0);
		}
		if (!isMultiLine) // check if line length > max line length
		{
			childValues_.reserve(size);
			addChildValues_ = true;
			int lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
			for (int index = 0; index < size; ++index) {
				if (has_comment_for_value(value[index])) {
					isMultiLine = true;
				}
				write_value(value[index]);
				lineLength += int(childValues_[index].length());
			}
			addChildValues_ = false;
			isMultiLine = isMultiLine || lineLength >= rightMargin_;
		}
		return isMultiLine;
	}

	void styled_writer::push_value(const std::string& value) {
		if (addChildValues_)
			childValues_.push_back(value);
		else
			document_ += value;
	}

	void styled_writer::write_indent() {
		if (!document_.empty()) {
			char last = document_[document_.length() - 1];
			if (last == ' ') // already indented
				return;
			if (last != '\n') // Comments may add new-line
				document_ += '\n';
		}
		document_ += indentString_;
	}

	void styled_writer::write_with_indent(const std::string& value) {
		write_indent();
		document_ += value;
	}

	void styled_writer::indent() { indentString_ += std::string(indentSize_, ' '); }

	void styled_writer::unindent() {
		assert(int(indentString_.size()) >= indentSize_);
		indentString_.resize(indentString_.size() - indentSize_);
	}

	void styled_writer::write_comment_before_value(const value& root) {
		if (!root.has_comment(comment_before))
			return;

		document_ += "\n";
		write_indent();
		const std::string& comment = root.get_comment(comment_before);
		std::string::const_iterator iter = comment.begin();
		while (iter != comment.end()) {
			document_ += *iter;
			if (*iter == '\n' &&
				(iter != comment.end() && *(iter + 1) == '/'))
				write_indent();
			++iter;
		}

		// Comments are stripped of trailing newlines, so add one here
		document_ += "\n";
	}

	void styled_writer::write_comment_after_value_on_same_line(const value& root) {
		if (root.has_comment(comment_after_on_same_line))
			document_ += " " + root.get_comment(comment_after_on_same_line);

		if (root.has_comment(comment_after)) {
			document_ += "\n";
			document_ += root.get_comment(comment_after);
			document_ += "\n";
		}
	}

	bool styled_writer::has_comment_for_value(const value& value) {
		return value.has_comment(comment_before) ||
			value.has_comment(comment_after_on_same_line) ||
			value.has_comment(comment_after);
	}

	// Class styled_stream_writer
	// //////////////////////////////////////////////////////////////////

	styled_stream_writer::styled_stream_writer(std::string indentation)
		: document_(NULL), rightMargin_(74), indentation_(indentation),
		addChildValues_() {}

	void styled_stream_writer::write(std::ostream& out, const value& root) {
		document_ = &out;
		addChildValues_ = false;
		indentString_ = "";
		indented_ = true;
		write_comment_before_value(root);
		if (!indented_) write_indent();
		indented_ = true;
		write_value(root);
		write_comment_after_value_on_same_line(root);
		*document_ << "\n";
		document_ = NULL; // Forget the stream, for safety.
	}

	void styled_stream_writer::write_value(const value& value) {
		switch (value.type()) {
		case null_value:
			push_value("null");
			break;
		case int_value:
			push_value(valueToString(value.as_largest_int()));
			break;
		case uint_value:
			push_value(valueToString(value.as_largest_uint()));
			break;
		case real_value:
			push_value(valueToString(value.as_double()));
			break;
		case string_value:
		{
			// Is NULL possible for value.string_?
			char const* str;
			char const* end;
			bool ok = value.get_string(&str, &end);
			if (ok) push_value(valueToQuotedStringN(str, static_cast<unsigned>(end - str)));
			else push_value("");
			break;
		}
		case boolean_value:
			push_value(valueToString(value.as_bool()));
			break;
		case array_value:
			write_array_value(value);
			break;
		case object_value: {
			value::Members members(value.get_member_names());
			if (members.empty())
				push_value("{}");
			else {
				write_with_indent("{");
				indent();
				value::Members::iterator it = members.begin();
				for (;;) {
					const std::string& name = *it;
					const json::value& childValue = value[name];
					write_comment_before_value(childValue);
					write_with_indent(valueToQuotedString(name.c_str()));
					*document_ << " : ";
					write_value(childValue);
					if (++it == members.end()) {
						write_comment_after_value_on_same_line(childValue);
						break;
					}
					*document_ << ",";
					write_comment_after_value_on_same_line(childValue);
				}
				unindent();
				write_with_indent("}");
			}
		} break;
		}
	}

	void styled_stream_writer::write_array_value(const value& value) {
		unsigned size = value.size();
		if (size == 0)
			push_value("[]");
		else {
			bool isArrayMultiLine = is_multine_array(value);
			if (isArrayMultiLine) {
				write_with_indent("[");
				indent();
				bool hasChildValue = !childValues_.empty();
				unsigned index = 0;
				for (;;) {
					const json::value& childValue = value[index];
					write_comment_before_value(childValue);
					if (hasChildValue)
						write_with_indent(childValues_[index]);
					else {
						if (!indented_) write_indent();
						indented_ = true;
						write_value(childValue);
						indented_ = false;
					}
					if (++index == size) {
						write_comment_after_value_on_same_line(childValue);
						break;
					}
					*document_ << ",";
					write_comment_after_value_on_same_line(childValue);
				}
				unindent();
				write_with_indent("]");
			}
			else // output on a single line
			{
				assert(childValues_.size() == size);
				*document_ << "[ ";
				for (unsigned index = 0; index < size; ++index) {
					if (index > 0)
						*document_ << ", ";
					*document_ << childValues_[index];
				}
				*document_ << " ]";
			}
		}
	}

	bool styled_stream_writer::is_multine_array(const value& value) {
		int size = value.size();
		bool isMultiLine = size * 3 >= rightMargin_;
		childValues_.clear();
		for (int index = 0; index < size && !isMultiLine; ++index) {
			const json::value& childValue = value[index];
			isMultiLine = ((childValue.is_array() || childValue.is_object()) &&
				childValue.size() > 0);
		}
		if (!isMultiLine) // check if line length > max line length
		{
			childValues_.reserve(size);
			addChildValues_ = true;
			int lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
			for (int index = 0; index < size; ++index) {
				if (has_comment_for_value(value[index])) {
					isMultiLine = true;
				}
				write_value(value[index]);
				lineLength += int(childValues_[index].length());
			}
			addChildValues_ = false;
			isMultiLine = isMultiLine || lineLength >= rightMargin_;
		}
		return isMultiLine;
	}

	void styled_stream_writer::push_value(const std::string& value) {
		if (addChildValues_)
			childValues_.push_back(value);
		else
			*document_ << value;
	}

	void styled_stream_writer::write_indent() {
		// blep intended this to look at the so-far-written string
		// to determine whether we are already indented, but
		// with a stream we cannot do that. So we rely on some saved state.
		// The caller checks indented_.
		*document_ << '\n' << indentString_;
	}

	void styled_stream_writer::write_with_indent(const std::string& value) {
		if (!indented_) write_indent();
		*document_ << value;
		indented_ = false;
	}

	void styled_stream_writer::indent() { indentString_ += indentation_; }

	void styled_stream_writer::unindent() {
		assert(indentString_.size() >= indentation_.size());
		indentString_.resize(indentString_.size() - indentation_.size());
	}

	void styled_stream_writer::write_comment_before_value(const value& root) {
		if (!root.has_comment(comment_before))
			return;

		if (!indented_) write_indent();
		const std::string& comment = root.get_comment(comment_before);
		std::string::const_iterator iter = comment.begin();
		while (iter != comment.end()) {
			*document_ << *iter;
			if (*iter == '\n' &&
				(iter != comment.end() && *(iter + 1) == '/'))
				// write_indent();  // would include newline
				*document_ << indentString_;
			++iter;
		}
		indented_ = false;
	}

	void styled_stream_writer::write_comment_after_value_on_same_line(const value& root) {
		if (root.has_comment(comment_after_on_same_line))
			*document_ << ' ' << root.get_comment(comment_after_on_same_line);

		if (root.has_comment(comment_after)) {
			write_indent();
			*document_ << root.get_comment(comment_after);
		}
		indented_ = false;
	}

	bool styled_stream_writer::has_comment_for_value(const value& value) {
		return value.has_comment(comment_before) ||
			value.has_comment(comment_after_on_same_line) ||
			value.has_comment(comment_after);
	}

	//////////////////////////
	// BuiltStyledStreamWriter

	/// Scoped enums are not available until C++11.
	struct CommentStyle {
		/// Decide whether to write comments.
		enum Enum {
			None,  ///< Drop all comments.
			Most,  ///< Recover odd behavior of previous versions (not implemented yet).
			All  ///< Keep all comments.
		};
	};

	struct BuiltStyledStreamWriter : public stream_writer
	{
		BuiltStyledStreamWriter(
			std::string const& indentation,
			CommentStyle::Enum cs,
			std::string const& colonSymbol,
			std::string const& nullSymbol,
			std::string const& endingLineFeedSymbol,
			bool useSpecialFloats);
		int write(value const& root, std::ostream* sout) override;
	private:
		void write_value(value const& value);
		void write_array_value(value const& value);
		bool is_multine_array(value const& value);
		void push_value(std::string const& value);
		void write_indent();
		void write_with_indent(std::string const& value);
		void indent();
		void unindent();
		void write_comment_before_value(value const& root);
		void write_comment_after_value_on_same_line(value const& root);
		static bool has_comment_for_value(const value& value);

		typedef std::vector<std::string> ChildValues;

		ChildValues childValues_;
		std::string indentString_;
		int rightMargin_;
		std::string indentation_;
		CommentStyle::Enum cs_;
		std::string colonSymbol_;
		std::string nullSymbol_;
		std::string endingLineFeedSymbol_;
		bool addChildValues_ : 1;
		bool indented_ : 1;
		bool useSpecialFloats_ : 1;
	};
	BuiltStyledStreamWriter::BuiltStyledStreamWriter(
		std::string const& indentation,
		CommentStyle::Enum cs,
		std::string const& colonSymbol,
		std::string const& nullSymbol,
		std::string const& endingLineFeedSymbol,
		bool useSpecialFloats)
		: rightMargin_(74)
		, indentation_(indentation)
		, cs_(cs)
		, colonSymbol_(colonSymbol)
		, nullSymbol_(nullSymbol)
		, endingLineFeedSymbol_(endingLineFeedSymbol)
		, addChildValues_(false)
		, indented_(false)
		, useSpecialFloats_(useSpecialFloats)
	{
	}
	int BuiltStyledStreamWriter::write(value const& root, std::ostream* sout)
	{
		sout_ = sout;
		addChildValues_ = false;
		indented_ = true;
		indentString_ = "";
		write_comment_before_value(root);
		if (!indented_) write_indent();
		indented_ = true;
		write_value(root);
		write_comment_after_value_on_same_line(root);
		*sout_ << endingLineFeedSymbol_;
		sout_ = NULL;
		return 0;
	}
	void BuiltStyledStreamWriter::write_value(value const& value) {
		switch (value.type()) {
		case null_value:
			push_value(nullSymbol_);
			break;
		case int_value:
			push_value(valueToString(value.as_largest_int()));
			break;
		case uint_value:
			push_value(valueToString(value.as_largest_uint()));
			break;
		case real_value:
			push_value(valueToString(value.as_double(), useSpecialFloats_));
			break;
		case string_value:
		{
			// Is NULL is possible for value.string_?
			char const* str;
			char const* end;
			bool ok = value.get_string(&str, &end);
			if (ok) push_value(valueToQuotedStringN(str, static_cast<unsigned>(end - str)));
			else push_value("");
			break;
		}
		case boolean_value:
			push_value(valueToString(value.as_bool()));
			break;
		case array_value:
			write_array_value(value);
			break;
		case object_value: {
			value::Members members(value.get_member_names());
			if (members.empty())
				push_value("{}");
			else {
				write_with_indent("{");
				indent();
				value::Members::iterator it = members.begin();
				for (;;) {
					std::string const& name = *it;
					json::value const& childValue = value[name];
					write_comment_before_value(childValue);
					write_with_indent(valueToQuotedStringN(name.data(), static_cast<unsigned>(name.length())));
					*sout_ << colonSymbol_;
					write_value(childValue);
					if (++it == members.end()) {
						write_comment_after_value_on_same_line(childValue);
						break;
					}
					*sout_ << ",";
					write_comment_after_value_on_same_line(childValue);
				}
				unindent();
				write_with_indent("}");
			}
		} break;
		}
	}

	void BuiltStyledStreamWriter::write_array_value(value const& value) {
		unsigned size = value.size();
		if (size == 0)
			push_value("[]");
		else {
			bool isMultiLine = (cs_ == CommentStyle::All) || is_multine_array(value);
			if (isMultiLine) {
				write_with_indent("[");
				indent();
				bool hasChildValue = !childValues_.empty();
				unsigned index = 0;
				for (;;) {
					json::value const& childValue = value[index];
					write_comment_before_value(childValue);
					if (hasChildValue)
						write_with_indent(childValues_[index]);
					else {
						if (!indented_) write_indent();
						indented_ = true;
						write_value(childValue);
						indented_ = false;
					}
					if (++index == size) {
						write_comment_after_value_on_same_line(childValue);
						break;
					}
					*sout_ << ",";
					write_comment_after_value_on_same_line(childValue);
				}
				unindent();
				write_with_indent("]");
			}
			else // output on a single line
			{
				assert(childValues_.size() == size);
				*sout_ << "[";
				if (!indentation_.empty()) *sout_ << " ";
				for (unsigned index = 0; index < size; ++index) {
					if (index > 0)
						*sout_ << ", ";
					*sout_ << childValues_[index];
				}
				if (!indentation_.empty()) *sout_ << " ";
				*sout_ << "]";
			}
		}
	}

	bool BuiltStyledStreamWriter::is_multine_array(value const& value) {
		int size = value.size();
		bool isMultiLine = size * 3 >= rightMargin_;
		childValues_.clear();
		for (int index = 0; index < size && !isMultiLine; ++index) {
			json::value const& childValue = value[index];
			isMultiLine = ((childValue.is_array() || childValue.is_object()) &&
				childValue.size() > 0);
		}
		if (!isMultiLine) // check if line length > max line length
		{
			childValues_.reserve(size);
			addChildValues_ = true;
			int lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
			for (int index = 0; index < size; ++index) {
				if (has_comment_for_value(value[index])) {
					isMultiLine = true;
				}
				write_value(value[index]);
				lineLength += int(childValues_[index].length());
			}
			addChildValues_ = false;
			isMultiLine = isMultiLine || lineLength >= rightMargin_;
		}
		return isMultiLine;
	}

	void BuiltStyledStreamWriter::push_value(std::string const& value) {
		if (addChildValues_)
			childValues_.push_back(value);
		else
			*sout_ << value;
	}

	void BuiltStyledStreamWriter::write_indent() {
		// blep intended this to look at the so-far-written string
		// to determine whether we are already indented, but
		// with a stream we cannot do that. So we rely on some saved state.
		// The caller checks indented_.

		if (!indentation_.empty()) {
			// In this case, drop newlines too.
			*sout_ << '\n' << indentString_;
		}
	}

	void BuiltStyledStreamWriter::write_with_indent(std::string const& value) {
		if (!indented_) write_indent();
		*sout_ << value;
		indented_ = false;
	}

	void BuiltStyledStreamWriter::indent() { indentString_ += indentation_; }

	void BuiltStyledStreamWriter::unindent() {
		assert(indentString_.size() >= indentation_.size());
		indentString_.resize(indentString_.size() - indentation_.size());
	}

	void BuiltStyledStreamWriter::write_comment_before_value(value const& root) {
		if (cs_ == CommentStyle::None) return;
		if (!root.has_comment(comment_before))
			return;

		if (!indented_) write_indent();
		const std::string& comment = root.get_comment(comment_before);
		std::string::const_iterator iter = comment.begin();
		while (iter != comment.end()) {
			*sout_ << *iter;
			if (*iter == '\n' &&
				(iter != comment.end() && *(iter + 1) == '/'))
				// write_indent();  // would write extra newline
				*sout_ << indentString_;
			++iter;
		}
		indented_ = false;
	}

	void BuiltStyledStreamWriter::write_comment_after_value_on_same_line(value const& root) {
		if (cs_ == CommentStyle::None) return;
		if (root.has_comment(comment_after_on_same_line))
			*sout_ << " " + root.get_comment(comment_after_on_same_line);

		if (root.has_comment(comment_after)) {
			write_indent();
			*sout_ << root.get_comment(comment_after);
		}
	}

	// static
	bool BuiltStyledStreamWriter::has_comment_for_value(const value& value) {
		return value.has_comment(comment_before) ||
			value.has_comment(comment_after_on_same_line) ||
			value.has_comment(comment_after);
	}

	///////////////
	// stream_writer

	stream_writer::stream_writer()
		: sout_(NULL)
	{
	}
	stream_writer::~stream_writer()
	{
	}
	stream_writer::factory::~factory()
	{}
	stream_writer_builder::stream_writer_builder()
	{
		set_defaults(&settings_);
	}
	stream_writer_builder::~stream_writer_builder()
	{}
	stream_writer* stream_writer_builder::new_stream_writer() const
	{
		std::string indentation = settings_["indentation"].as_string();
		std::string cs_str = settings_["commentStyle"].as_string();
		bool eyc = settings_["enable_YAML_compatibility"].as_bool();
		bool dnp = settings_["dropNullPlaceholders"].as_bool();
		bool usf = settings_["useSpecialFloats"].as_bool();
		CommentStyle::Enum cs = CommentStyle::All;
		if (cs_str == "All") {
			cs = CommentStyle::All;
		}
		else if (cs_str == "None") {
			cs = CommentStyle::None;
		}
		else {
			throw_runtime_error("commentStyle must be 'All' or 'None'");
		}
		std::string colonSymbol = " : ";
		if (eyc) {
			colonSymbol = ": ";
		}
		else if (indentation.empty()) {
			colonSymbol = ":";
		}
		std::string nullSymbol = "null";
		if (dnp) {
			nullSymbol = "";
		}
		std::string endingLineFeedSymbol = "";
		return new BuiltStyledStreamWriter(
			indentation, cs,
			colonSymbol, nullSymbol, endingLineFeedSymbol, usf);
	}
	static void getValidWriterKeys(std::set<std::string>* valid_keys)
	{
		valid_keys->clear();
		valid_keys->insert("indentation");
		valid_keys->insert("commentStyle");
		valid_keys->insert("enable_YAML_compatibility");
		valid_keys->insert("dropNullPlaceholders");
		valid_keys->insert("useSpecialFloats");
	}
	bool stream_writer_builder::validate(json::value* invalid) const
	{
		json::value my_invalid;
		if (!invalid) invalid = &my_invalid;  // so we do not need to test for NULL
		json::value& inv = *invalid;
		std::set<std::string> valid_keys;
		getValidWriterKeys(&valid_keys);
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
	value& stream_writer_builder::operator[](std::string key)
	{
		return settings_[key];
	}
	// static
	void stream_writer_builder::set_defaults(json::value* settings)
	{
		//! [StreamWriterBuilderDefaults]
		(*settings)["commentStyle"] = "All";
		(*settings)["indentation"] = "\t";
		(*settings)["enable_YAML_compatibility"] = false;
		(*settings)["dropNullPlaceholders"] = false;
		(*settings)["useSpecialFloats"] = false;
		//! [StreamWriterBuilderDefaults]
	}

	std::string writeString(stream_writer::factory const& builder, value const& root) {
		std::ostringstream sout;
		StreamWriterPtr const writer(builder.new_stream_writer());
		writer->write(root, &sout);
		return sout.str();
	}

	std::ostream& operator<<(std::ostream& sout, value const& root) {
		stream_writer_builder builder;
		StreamWriterPtr const writer(builder.new_stream_writer());
		writer->write(root, &sout);
		return sout;
	}

} // namespace json
