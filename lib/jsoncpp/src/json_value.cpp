// Copyright 2011 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
#include <json/assertions.h>
#include <json/value.h>
#include <json/writer.h>
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <math.h>
#include <sstream>
#include <utility>
#include <cstring>
#include <cassert>
#ifdef JSON_USE_CPPTL
#include <cpptl/conststring.h>
#endif
#include <cstddef> // size_t
#include <algorithm> // min()

#define JSON_ASSERT_UNREACHABLE assert(false)

namespace json {

	// This is a walkaround to avoid the static initialization of value::null.
	// kNull must be word-aligned to avoid crashing on ARM.  We use an alignment of
	// 8 (instead of 4) as a bit of future-proofing.
#if defined(__ARMEL__)
#define ALIGNAS(byte_alignment) __attribute__((aligned(byte_alignment)))
#else
#define ALIGNAS(byte_alignment)
#endif
	static const unsigned char ALIGNAS(8) kNull[sizeof(value)] = { 0 };
	const unsigned char& kNullRef = kNull[0];
	const value& value::null = reinterpret_cast<const value&>(kNullRef);
	const value& value::nullRef = null;

	const int value::min_int = int(~(uint(-1) / 2));
	const int value::max_int = int(uint(-1) / 2);
	const uint value::maxUInt = uint(-1);
#if defined(JSON_HAS_INT64)
	const int64 value::min_int64 = int64(~(uint64(-1) / 2));
	const int64 value::max_int64 = int64(uint64(-1) / 2);
	const uint64 value::max_uint64 = uint64(-1);
	// The constant is hard-coded because some compiler have trouble
	// converting value::max_uint64 to a double correctly (AIX/xlC).
	// Assumes that uint64 is a 64 bits integer.
	static const double max_uint64_as_double = 18446744073709551615.0;
#endif // defined(JSON_HAS_INT64)
	const largest_int value::min_largest_int = largest_int(~(largest_uint(-1) / 2));
	const largest_int value::max_largest_int = largest_int(largest_uint(-1) / 2);
	const largest_uint value::max_largest_uint = largest_uint(-1);

#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
	template <typename T, typename U>
	static inline bool in_range(double d, T min, U max)
	{
		return d >= min && d <= max;
	}
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
	static inline double integer_to_double(json::uint64 value)
	{
		return static_cast<double>(int64(value / 2)) * 2.0 + int64(value & 1);
	}

	template <typename T> static inline double integer_to_double(T value)
	{
		return static_cast<double>(value);
	}

	template <typename T, typename U>
	static inline bool in_range(double d, T min, U max)
	{
		return d >= integer_to_double(min) && d <= integer_to_double(max);
	}
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)

	/** Duplicates the specified string value.
	 * @param value Pointer to the string to duplicate. Must be zero-terminated if
	 *              length is "unknown".
	 * @param length Length of the value. if equals to unknown, then it will be
	 *               computed using strlen(value).
	 * @return Pointer on the duplicate instance of string.
	 */
	static inline char* duplicateStringValue(const char* value, size_t length)
	{
		// Avoid an integer overflow in the call to malloc below by limiting length
		// to a sane value.
		if (length >= (size_t)value::max_int)
			length = value::max_int - 1;

		char* newString = static_cast<char*>(malloc(length + 1));
		if (newString == NULL)
			throw_runtime_error("in json::value::duplicateStringValue(): Failed to allocate string value buffer");
		
		memcpy(newString, value, length);
		newString[length] = 0;
		return newString;
	}

	/* Record the length as a prefix.
	 */
	static inline char* duplicateAndPrefixStringValue(const char* value, unsigned int length)
	{
		// Avoid an integer overflow in the call to malloc below by limiting length
		// to a sane value.
		JSON_ASSERT_MESSAGE(length <= (unsigned)value::max_int - sizeof(unsigned) - 1U,
			"in json::value::duplicateAndPrefixStringValue(): "
			"length too big for prefixing");
		unsigned actualLength = length + static_cast<unsigned>(sizeof(unsigned)) + 1U;
		char* newString = static_cast<char*>(malloc(actualLength));
		if (newString == 0)
		{
			throw_runtime_error("in json::value::duplicateAndPrefixStringValue(): Failed to allocate string value buffer");
		}
		*reinterpret_cast<unsigned*>(newString) = length;
		memcpy(newString + sizeof(unsigned), value, length);
		newString[actualLength - 1U] = 0; // to avoid buffer over-run accidents by users later
		return newString;
	}
	inline static void decodePrefixedString(bool isPrefixed, char const* prefixed, unsigned* length, char const** value)
	{
		if (!isPrefixed)
		{
			*length = static_cast<unsigned>(strlen(prefixed));
			*value = prefixed;
		}
		else
		{
			*length = *reinterpret_cast<unsigned const*>(prefixed);
			*value = prefixed + sizeof(unsigned);
		}
	}
	/** Free the string duplicated by duplicateStringValue()/duplicateAndPrefixStringValue().
	 */
	static inline void releaseStringValue(char* value) { free(value); }

} // namespace json

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// ValueInternals...
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
#if !defined(JSON_IS_AMALGAMATION)

#include "json_valueiterator.inl"
#endif // if !defined(JSON_IS_AMALGAMATION)

namespace json
{

	exception::exception(std::string const& msg)
		: msg_(msg)
	{ }

	exception::~exception() throw()
	{ }

	char const* exception::what() const throw()
	{
		return msg_.c_str();
	}

	runtime_error::runtime_error(std::string const& msg)
		: exception(msg)
	{ }

	logic_error::logic_error(std::string const& msg)
		: exception(msg)
	{ }

	void throw_runtime_error(std::string const& msg)
	{
		throw runtime_error(msg);
	}

	void throw_logic_error(std::string const& msg)
	{
		throw logic_error(msg);
	}

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class value::comment_info
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	value::comment_info::comment_info() : comment_(0) {}

	value::comment_info::~comment_info()
	{
		if (comment_)
			releaseStringValue(comment_);
	}

	void value::comment_info::set_comment(const char* text, size_t len)
	{
		if (comment_)
		{
			releaseStringValue(comment_);
			comment_ = 0;
		}
		JSON_ASSERT(text != 0);
		JSON_ASSERT_MESSAGE(text[0] == '\0' || text[0] == '/',
			"in json::value::set_comment(): Comments must start with /");
		// It seems that /**/ style comments are acceptable as well.
		comment_ = duplicateStringValue(text, len);
	}

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class value::czstring
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	// Notes: policy_ indicates if the string was allocated when
	// a string is stored.

	value::czstring::czstring(array_index aindex) : cstr_(0), index_(aindex)
	{ }

	value::czstring::czstring(char const* str, unsigned ulength, duplication_policy allocate)
		: cstr_(str)
	{
		// allocate != duplicate
		storage_.policy_ = allocate & 0x3;
		storage_.length_ = ulength & 0x3FFFFFFF;
	}

	value::czstring::czstring(const czstring& other)
		: cstr_(other.storage_.policy_ != no_duplication && other.cstr_ != 0
				? duplicateStringValue(other.cstr_, other.storage_.length_)
				: other.cstr_)
	{
		storage_.policy_ = (other.cstr_
			? (static_cast<duplication_policy>(other.storage_.policy_) == no_duplication
			? no_duplication : duplicate)
			: static_cast<duplication_policy>(other.storage_.policy_));
		storage_.length_ = other.storage_.length_;
	}

	value::czstring::~czstring()
	{
		if (cstr_ && storage_.policy_ == duplicate)
			releaseStringValue(const_cast<char*>(cstr_));
	}

	void value::czstring::swap(czstring& other)
	{
		std::swap(cstr_, other.cstr_);
		std::swap(index_, other.index_);
	}

	value::czstring& value::czstring::operator=(czstring other)
	{
		swap(other);
		return *this;
	}

	bool value::czstring::operator<(const czstring& other) const
	{
		if (!cstr_) return index_ < other.index_;
		//return strcmp(cstr_, other.cstr_) < 0;
		// Assume both are strings.
		unsigned this_len = this->storage_.length_;
		unsigned other_len = other.storage_.length_;
		unsigned min_len = std::min(this_len, other_len);
		int comp = memcmp(this->cstr_, other.cstr_, min_len);
		if (comp < 0) return true;
		if (comp > 0) return false;
		return (this_len < other_len);
	}

	bool value::czstring::operator==(const czstring& other) const
	{
		if (!cstr_) return index_ == other.index_;
		//return strcmp(cstr_, other.cstr_) == 0;
		// Assume both are strings.
		unsigned this_len = this->storage_.length_;
		unsigned other_len = other.storage_.length_;
		if (this_len != other_len) return false;
		int comp = memcmp(this->cstr_, other.cstr_, this_len);
		return comp == 0;
	}

	array_index value::czstring::index() const { return index_; }

	//const char* value::czstring::c_str() const { return cstr_; }
	const char* value::czstring::data() const { return cstr_; }
	unsigned value::czstring::length() const { return storage_.length_; }
	bool value::czstring::isStaticString() const { return storage_.policy_ == no_duplication; }

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class value::value
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	/*! \internal Default constructor initialization must be equivalent to:
	 * memset( this, 0, sizeof(value) )
	 * This optimization is used in ValueInternalMap fast allocator.
	 */
	value::value(value_type vtype)
	{
		init_basic(vtype);
		switch (vtype) {
		case null_value:
			break;
		case int_value:
		case uint_value:
			value_.int_ = 0;
			break;
		case real_value:
			value_.real_ = 0.0;
			break;
		case string_value:
			value_.string_ = 0;
			break;
		case array_value:
		case object_value:
			value_.map_ = new object_values();
			break;
		case boolean_value:
			value_.bool_ = false;
			break;
		default:
			JSON_ASSERT_UNREACHABLE;
		}
	}

	value::value(int value)
	{
		init_basic(int_value);
		value_.int_ = value;
	}

	value::value(uint value)
	{
		init_basic(uint_value);
		value_.uint_ = value;
	}
#if defined(JSON_HAS_INT64)
	value::value(int64 value)
	{
		init_basic(int_value);
		value_.int_ = value;
	}
	value::value(uint64 value)
	{
		init_basic(uint_value);
		value_.uint_ = value;
	}
#endif // defined(JSON_HAS_INT64)

	value::value(double value)
	{
		init_basic(real_value);
		value_.real_ = value;
	}

	value::value(const char* value)
	{
		init_basic(string_value, true);
		value_.string_ = duplicateAndPrefixStringValue(value, static_cast<unsigned>(strlen(value)));
	}

	value::value(const char* beginValue, const char* endValue)
	{
		init_basic(string_value, true);
		value_.string_ =
			duplicateAndPrefixStringValue(beginValue, static_cast<unsigned>(endValue - beginValue));
	}

	value::value(const std::string& value) 
	{
		init_basic(string_value, true);
		value_.string_ =
			duplicateAndPrefixStringValue(value.data(), static_cast<unsigned>(value.length()));
	}

	value::value(const static_string& value)
	{
		init_basic(string_value);
		value_.string_ = const_cast<char*>(value.c_str());
	}

#ifdef JSON_USE_CPPTL
	value::value(const CppTL::ConstString& value)
	{
		init_basic(string_value, true);
		value_.string_ = duplicateAndPrefixStringValue(value, static_cast<unsigned>(value.length()));
	}
#endif

	value::value(bool value)
	{
		init_basic(boolean_value);
		value_.bool_ = value;
	}

	value::value(value const& other)
		: type_(other.type_)
		, allocated_(false)
		, comments_(0)
		, start_(other.start_)
		, limit_(other.limit_)
	{
		switch (type_)
		{
		case null_value:
		case int_value:
		case uint_value:
		case real_value:
		case boolean_value:
			value_ = other.value_;
			break;
		case string_value:
			if (other.value_.string_ && other.allocated_)
			{
				unsigned len;
				char const* str;
				decodePrefixedString(other.allocated_, other.value_.string_,
					&len, &str);
				value_.string_ = duplicateAndPrefixStringValue(str, len);
				allocated_ = true;
			}
			else
			{
				value_.string_ = other.value_.string_;
				allocated_ = false;
			}
			break;
		case array_value:
		case object_value:
			value_.map_ = new object_values(*other.value_.map_);
			break;
		default:
			JSON_ASSERT_UNREACHABLE;
		}

		if (other.comments_)
		{
			comments_ = new comment_info[number_of_comment_placement];
			for (int comment = 0; comment < number_of_comment_placement; ++comment)
			{
				const comment_info& otherComment = other.comments_[comment];
				if (otherComment.comment_)
					comments_[comment].set_comment(otherComment.comment_, strlen(otherComment.comment_));
			}
		}
	}

	value::~value()
	{
		switch (type_)
		{
		case null_value:
		case int_value:
		case uint_value:
		case real_value:
		case boolean_value:
			break;
		case string_value:
			if (allocated_)
				releaseStringValue(value_.string_);
			break;
		case array_value:
		case object_value:
			delete value_.map_;
			break;
		default:
			JSON_ASSERT_UNREACHABLE;
		}

		if (comments_)
			delete[] comments_;
	}

	value& value::operator=(value other)
	{
		swap(other);
		return *this;
	}

	void value::swap_payload(value& other)
	{
		value_type temp = type_;
		type_ = other.type_;
		other.type_ = temp;
		std::swap(value_, other.value_);
		int temp2 = allocated_;
		allocated_ = other.allocated_;
		other.allocated_ = temp2 & 0x1;
	}

	void value::swap(value& other)
	{
		swap_payload(other);
		std::swap(comments_, other.comments_);
		std::swap(start_, other.start_);
		std::swap(limit_, other.limit_);
	}

	value_type value::type() const { return type_; }

	int value::compare(const value& other) const
	{
		if (*this < other)
			return -1;
		if (*this > other)
			return 1;
		return 0;
	}

	bool value::operator<(const value& other) const
	{
		int typeDelta = type_ - other.type_;
		if (typeDelta)
			return typeDelta < 0 ? true : false;
		switch (type_)
		{
		case null_value:
			return false;
		case int_value:
			return value_.int_ < other.value_.int_;
		case uint_value:
			return value_.uint_ < other.value_.uint_;
		case real_value:
			return value_.real_ < other.value_.real_;
		case boolean_value:
			return value_.bool_ < other.value_.bool_;
		case string_value:
		{
			if ((value_.string_ == 0) || (other.value_.string_ == 0))
			{
				if (other.value_.string_) return true;
				else return false;
			}
			unsigned this_len;
			unsigned other_len;
			char const* this_str;
			char const* other_str;
			decodePrefixedString(this->allocated_, this->value_.string_, &this_len, &this_str);
			decodePrefixedString(other.allocated_, other.value_.string_, &other_len, &other_str);
			unsigned min_len = std::min(this_len, other_len);
			int comp = memcmp(this_str, other_str, min_len);
			if (comp < 0) return true;
			if (comp > 0) return false;
			return (this_len < other_len);
		}
		case array_value:
		case object_value:
		{
			int delta = int(value_.map_->size() - other.value_.map_->size());
			if (delta)
				return delta < 0;
			return (*value_.map_) < (*other.value_.map_);
		}
		default:
			JSON_ASSERT_UNREACHABLE;
		}
		return false; // unreachable
	}

	bool value::operator<=(const value& other) const { return !(other < *this); }

	bool value::operator>=(const value& other) const { return !(*this < other); }

	bool value::operator>(const value& other) const { return other < *this; }

	bool value::operator==(const value& other) const
	{
		// if ( type_ != other.type_ )
		// GCC 2.95.3 says:
		// attempt to take address of bit-field structure member `json::value::type_'
		// Beats me, but a temp solves the problem.
		int temp = other.type_;
		if (type_ != temp)
			return false;
		switch (type_)
		{
		case null_value:
			return true;
		case int_value:
			return value_.int_ == other.value_.int_;
		case uint_value:
			return value_.uint_ == other.value_.uint_;
		case real_value:
			return value_.real_ == other.value_.real_;
		case boolean_value:
			return value_.bool_ == other.value_.bool_;
		case string_value:
		{
			if ((value_.string_ == 0) || (other.value_.string_ == 0))
			{
				return (value_.string_ == other.value_.string_);
			}
			unsigned this_len;
			unsigned other_len;
			char const* this_str;
			char const* other_str;
			decodePrefixedString(this->allocated_, this->value_.string_, &this_len, &this_str);
			decodePrefixedString(other.allocated_, other.value_.string_, &other_len, &other_str);
			if (this_len != other_len) return false;
			int comp = memcmp(this_str, other_str, this_len);
			return comp == 0;
		}
		case array_value:
		case object_value:
			return value_.map_->size() == other.value_.map_->size() &&
				(*value_.map_) == (*other.value_.map_);
		default:
			JSON_ASSERT_UNREACHABLE;
		}
		return false; // unreachable
	}

	bool value::operator!=(const value& other) const { return !(*this == other); }

	const char* value::as_cstring() const
	{
		JSON_ASSERT_MESSAGE(type_ == string_value,
			"in json::value::as_cstring(): requires string_value");
		if (value_.string_ == 0) return 0;
		unsigned this_len;
		char const* this_str;
		decodePrefixedString(this->allocated_, this->value_.string_, &this_len, &this_str);
		return this_str;
	}

	bool value::get_string(char const** str, char const** cend) const
	{
		if (type_ != string_value) return false;
		if (value_.string_ == 0) return false;
		unsigned length;
		decodePrefixedString(this->allocated_, this->value_.string_, &length, str);
		*cend = *str + length;
		return true;
	}

	std::string value::as_string() const
	{
		switch (type_) {
		case null_value:
			return "";
		case string_value:
		{
			if (value_.string_ == 0) return "";
			unsigned this_len;
			char const* this_str;
			decodePrefixedString(this->allocated_, this->value_.string_, &this_len, &this_str);
			return std::string(this_str, this_len);
		}
		case boolean_value:
			return value_.bool_ ? "true" : "false";
		case int_value:
			return valueToString(value_.int_);
		case uint_value:
			return valueToString(value_.uint_);
		case real_value:
			return valueToString(value_.real_);
		default:
			JSON_FAIL_MESSAGE("Type is not convertible to string");
		}
	}

#ifdef JSON_USE_CPPTL
	CppTL::ConstString value::asConstString() const
	{
		unsigned len;
		char const* str;
		decodePrefixedString(allocated_, value_.string_,
			&len, &str);
		return CppTL::ConstString(str, len);
	}
#endif

	int value::as_int() const
	{
		switch (type_)
		{
		case int_value:
			JSON_ASSERT_MESSAGE(is_int(), "largest_int out of int range");
			return int(value_.int_);
		case uint_value:
			JSON_ASSERT_MESSAGE(is_int(), "largest_uint out of int range");
			return int(value_.uint_);
		case real_value:
			JSON_ASSERT_MESSAGE(in_range(value_.real_, min_int, max_int),
				"double out of int range");
			return int(value_.real_);
		case null_value:
			return 0;
		case boolean_value:
			return value_.bool_ ? 1 : 0;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("value is not convertible to int.");
	}

	value::uint value::as_uint() const
	{
		switch (type_)
		{
		case int_value:
			JSON_ASSERT_MESSAGE(is_uint(), "largest_int out of uint range");
			return uint(value_.int_);
		case uint_value:
			JSON_ASSERT_MESSAGE(is_uint(), "largest_uint out of uint range");
			return uint(value_.uint_);
		case real_value:
			JSON_ASSERT_MESSAGE(in_range(value_.real_, 0, maxUInt),
				"double out of uint range");
			return uint(value_.real_);
		case null_value:
			return 0;
		case boolean_value:
			return value_.bool_ ? 1 : 0;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("value is not convertible to uint.");
	}

#if defined(JSON_HAS_INT64)

	value::int64 value::as_int64() const
	{
		switch (type_)
		{
		case int_value:
			return int64(value_.int_);
		case uint_value:
			JSON_ASSERT_MESSAGE(is_int64(), "largest_uint out of int64 range");
			return int64(value_.uint_);
		case real_value:
			JSON_ASSERT_MESSAGE(in_range(value_.real_, min_int64, max_int64),
				"double out of int64 range");
			return int64(value_.real_);
		case null_value:
			return 0;
		case boolean_value:
			return value_.bool_ ? 1 : 0;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("value is not convertible to int64.");
	}

	value::uint64 value::as_uint64() const
	{
		switch (type_)
		{
		case int_value:
			JSON_ASSERT_MESSAGE(is_uint64(), "largest_int out of uint64 range");
			return uint64(value_.int_);
		case uint_value:
			return uint64(value_.uint_);
		case real_value:
			JSON_ASSERT_MESSAGE(in_range(value_.real_, 0, max_uint64),
				"double out of uint64 range");
			return uint64(value_.real_);
		case null_value:
			return 0;
		case boolean_value:
			return value_.bool_ ? 1 : 0;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("value is not convertible to uint64.");
	}
#endif // if defined(JSON_HAS_INT64)

	largest_int value::as_largest_int() const
	{
#if defined(JSON_NO_INT64)
		return as_int();
#else
		return as_int64();
#endif
	}

	largest_uint value::as_largest_uint() const
	{
#if defined(JSON_NO_INT64)
		return as_uint();
#else
		return as_uint64();
#endif
	}

	double value::as_double() const
	{
		switch (type_)
		{
		case int_value:
			return static_cast<double>(value_.int_);
		case uint_value:
#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
			return static_cast<double>(value_.uint_);
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
			return integer_to_double(value_.uint_);
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
		case real_value:
			return value_.real_;
		case null_value:
			return 0.0;
		case boolean_value:
			return value_.bool_ ? 1.0 : 0.0;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("value is not convertible to double.");
	}

	float value::as_float() const
	{
		switch (type_)
		{
		case int_value:
			return static_cast<float>(value_.int_);
		case uint_value:
#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
			return static_cast<float>(value_.uint_);
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
			return integer_to_double(value_.uint_);
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
		case real_value:
			return static_cast<float>(value_.real_);
		case null_value:
			return 0.0;
		case boolean_value:
			return value_.bool_ ? 1.0f : 0.0f;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("value is not convertible to float.");
	}

	bool value::as_bool() const
	{
		switch (type_)
		{
		case boolean_value:
			return value_.bool_;
		case null_value:
			return false;
		case int_value:
			return value_.int_ ? true : false;
		case uint_value:
			return value_.uint_ ? true : false;
		case real_value:
			// This is kind of strange. Not recommended.
			return (value_.real_ != 0.0) ? true : false;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("value is not convertible to bool.");
	}

	bool value::is_convertible_to(value_type other) const
	{
		switch (other) 
		{
		case null_value:
			return (is_numeric() && as_double() == 0.0) ||
				(type_ == boolean_value && value_.bool_ == false) ||
				(type_ == string_value && as_string() == "") ||
				(type_ == array_value && value_.map_->size() == 0) ||
				(type_ == object_value && value_.map_->size() == 0) ||
				type_ == null_value;
		case int_value:
			return is_int() ||
				(type_ == real_value && in_range(value_.real_, min_int, max_int)) ||
				type_ == boolean_value || type_ == null_value;
		case uint_value:
			return is_uint() ||
				(type_ == real_value && in_range(value_.real_, 0, maxUInt)) ||
				type_ == boolean_value || type_ == null_value;
		case real_value:
			return is_numeric() || type_ == boolean_value || type_ == null_value;
		case boolean_value:
			return is_numeric() || type_ == boolean_value || type_ == null_value;
		case string_value:
			return is_numeric() || type_ == boolean_value || type_ == string_value ||
				type_ == null_value;
		case array_value:
			return type_ == array_value || type_ == null_value;
		case object_value:
			return type_ == object_value || type_ == null_value;
		}
		JSON_ASSERT_UNREACHABLE;
		return false;
	}

	/// Number of values in array or object
	array_index value::size() const
	{
		switch (type_)
		{
		case null_value:
		case int_value:
		case uint_value:
		case real_value:
		case boolean_value:
		case string_value:
			return 0;
		case array_value: // size of the array is highest index + 1
			if (!value_.map_->empty())
			{
				object_values::const_iterator itLast = value_.map_->end();
				--itLast;
				return (*itLast).first.index() + 1;
			}
			return 0;
		case object_value:
			return array_index(value_.map_->size());
		}
		JSON_ASSERT_UNREACHABLE;
		return 0; // unreachable;
	}

	bool value::empty() const
	{
		if (is_null() || is_array() || is_object())
			return size() == 0u;
		else
			return false;
	}

	bool value::operator!() const { return is_null(); }

	void value::clear()
	{
		JSON_ASSERT_MESSAGE(type_ == null_value || type_ == array_value || type_ == object_value,
			"in json::value::clear(): requires complex value");
		start_ = 0;
		limit_ = 0;
		switch (type_)
		{
		case array_value:
		case object_value:
			value_.map_->clear();
			break;
		default:
			break;
		}
	}

	void value::resize(array_index newSize)
	{
		JSON_ASSERT_MESSAGE(type_ == null_value || type_ == array_value,
			"in json::value::resize(): requires array_value");
		if (type_ == null_value)
			*this = value(array_value);
		array_index oldSize = size();
		if (newSize == 0)
			clear();
		else if (newSize > oldSize)
			(*this)[newSize - 1];
		else {
			for (array_index index = newSize; index < oldSize; ++index) {
				value_.map_->erase(index);
			}
			assert(size() == newSize);
		}
	}

	value& value::operator[](array_index index)
	{
		JSON_ASSERT_MESSAGE(type_ == null_value || type_ == array_value,
			"in json::value::operator[](array_index): requires array_value");
		if (type_ == null_value)
			*this = value(array_value);
		czstring key(index);
		object_values::iterator it = value_.map_->lower_bound(key);
		if (it != value_.map_->end() && (*it).first == key)
			return (*it).second;

		object_values::value_type defaultValue(key, nullRef);
		it = value_.map_->insert(it, defaultValue);
		return (*it).second;
	}

	value& value::operator[](int index)
	{
		JSON_ASSERT_MESSAGE(
			index >= 0,
			"in json::value::operator[](int index): index cannot be negative");
		return (*this)[array_index(index)];
	}

	const value& value::operator[](array_index index) const
	{
		JSON_ASSERT_MESSAGE(
			type_ == null_value || type_ == array_value,
			"in json::value::operator[](array_index)const: requires array_value");
		if (type_ == null_value)
			return nullRef;
		czstring key(index);
		object_values::const_iterator it = value_.map_->find(key);
		if (it == value_.map_->end())
			return nullRef;
		return (*it).second;
	}

	const value& value::operator[](int index) const
	{
		JSON_ASSERT_MESSAGE(
			index >= 0,
			"in json::value::operator[](int index) const: index cannot be negative");
		return (*this)[array_index(index)];
	}

	void value::init_basic(value_type vtype, bool allocated)
	{
		type_ = vtype;
		allocated_ = allocated;
		comments_ = 0;
		start_ = 0;
		limit_ = 0;
	}

	// Access an object value by name, create a null member if it does not exist.
	// @pre Type of '*this' is object or null.
	// @param key is null-terminated.
	value& value::resolve_reference(const char* key)
	{
		JSON_ASSERT_MESSAGE(type_ == null_value || type_ == object_value,
			"in json::value::resolve_reference(): requires object_value");
		if (type_ == null_value)
			*this = value(object_value);
		czstring actualKey(
			key, static_cast<unsigned>(strlen(key)), czstring::no_duplication); // NOTE!
		object_values::iterator it = value_.map_->lower_bound(actualKey);
		if (it != value_.map_->end() && (*it).first == actualKey)
			return (*it).second;

		object_values::value_type defaultValue(actualKey, nullRef);
		it = value_.map_->insert(it, defaultValue);
		value& value = (*it).second;
		return value;
	}

	// @param key is not null-terminated.
	value& value::resolve_reference(char const* key, char const* cend)
	{
		JSON_ASSERT_MESSAGE(type_ == null_value || type_ == object_value,
			"in json::value::resolve_reference(key, end): requires object_value");
		if (type_ == null_value)
			*this = value(object_value);
		czstring actualKey(
			key, static_cast<unsigned>(cend - key), czstring::duplicate_on_copy);
		object_values::iterator it = value_.map_->lower_bound(actualKey);
		if (it != value_.map_->end() && (*it).first == actualKey)
			return (*it).second;

		object_values::value_type defaultValue(actualKey, nullRef);
		it = value_.map_->insert(it, defaultValue);
		value& value = (*it).second;
		return value;
	}

	value value::get(array_index index, const value& defaultValue) const {
		const value* value = &((*this)[index]);
		return value == &nullRef ? defaultValue : *value;
	}

	bool value::is_valid_index(array_index index) const { return index < size(); }

	value const* value::find(char const* key, char const* cend) const
	{
		JSON_ASSERT_MESSAGE(
			type_ == null_value || type_ == object_value,
			"in json::value::find(key, end, found): requires object_value or null_value");
		if (type_ == null_value) return NULL;
		czstring actualKey(key, static_cast<unsigned>(cend - key), czstring::no_duplication);
		object_values::const_iterator it = value_.map_->find(actualKey);
		if (it == value_.map_->end()) return NULL;
		return &(*it).second;
	}

	const value& value::operator[](const char* key) const
	{
		value const* found = find(key, key + strlen(key));
		if (!found) return nullRef;
		return *found;
	}

	value const& value::operator[](std::string const& key) const
	{
		value const* found = find(key.data(), key.data() + key.length());
		if (!found) return nullRef;
		return *found;
	}

	value& value::operator[](const char* key)
	{
		return resolve_reference(key, key + strlen(key));
	}

	value& value::operator[](const std::string& key)
	{
		return resolve_reference(key.data(), key.data() + key.length());
	}

	value& value::operator[](const static_string& key) {
		return resolve_reference(key.c_str());
	}

#ifdef JSON_USE_CPPTL
	value& value::operator[](const CppTL::ConstString& key)
	{
		return resolve_reference(key.c_str(), key.end_c_str());
	}

	value const& value::operator[](CppTL::ConstString const& key) const
	{
		value const* found = find(key.c_str(), key.end_c_str());
		if (!found) return nullRef;
		return *found;
	}
#endif

	value& value::append(const value& value) { return (*this)[size()] = value; }

	value value::get(char const* key, char const* cend, value const& defaultValue) const
	{
		value const* found = find(key, cend);
		return !found ? defaultValue : *found;
	}

	value value::get(char const* key, value const& defaultValue) const
	{
		return get(key, key + strlen(key), defaultValue);
	}

	value value::get(std::string const& key, value const& defaultValue) const
	{
		return get(key.data(), key.data() + key.length(), defaultValue);
	}


	bool value::remove_member(const char* key, const char* cend, value* removed)
	{
		if (type_ != object_value)
		{
			return false;
		}
		czstring actualKey(key, static_cast<unsigned>(cend - key), czstring::no_duplication);
		object_values::iterator it = value_.map_->find(actualKey);
		if (it == value_.map_->end())
			return false;
		*removed = it->second;
		value_.map_->erase(it);
		return true;
	}

	bool value::remove_member(const char* key, value* removed)
	{
		return remove_member(key, key + strlen(key), removed);
	}

	bool value::remove_member(std::string const& key, value* removed)
	{
		return remove_member(key.data(), key.data() + key.length(), removed);
	}

	value value::remove_member(const char* key)
	{
		JSON_ASSERT_MESSAGE(type_ == null_value || type_ == object_value,
			"in json::value::remove_member(): requires object_value");
		if (type_ == null_value)
			return nullRef;

		value removed;  // null
		remove_member(key, key + strlen(key), &removed);
		return removed; // still null if remove_member() did nothing
	}

	value value::remove_member(const std::string& key)
	{
		return remove_member(key.c_str());
	}

	bool value::remove_index(array_index index, value* removed)
	{
		if (type_ != array_value)
			return false;
		
		czstring key(index);
		object_values::iterator it = value_.map_->find(key);
		if (it == value_.map_->end())
			return false;
		
		*removed = it->second;
		array_index oldSize = size();
		// shift left all items left, into the place of the "removed"
		for (array_index i = index; i < (oldSize - 1); ++i)
		{
			czstring keey(i);
			(*value_.map_)[keey] = (*this)[i + 1];
		}
		// erase the last one ("leftover")
		czstring keyLast(oldSize - 1);
		object_values::iterator itLast = value_.map_->find(keyLast);
		value_.map_->erase(itLast);
		return true;
	}

#ifdef JSON_USE_CPPTL
	value value::get(const CppTL::ConstString& key, const value& defaultValue) const
	{
		return get(key.c_str(), key.end_c_str(), defaultValue);
	}
#endif

	bool value::is_member(char const* key, char const* cend) const
	{
		value const* value = find(key, cend);
		return NULL != value;
	}

	bool value::is_member(char const* key) const
	{
		return is_member(key, key + strlen(key));
	}

	bool value::is_member(std::string const& key) const
	{
		return is_member(key.data(), key.data() + key.length());
	}

#ifdef JSON_USE_CPPTL
	bool value::is_member(const CppTL::ConstString& key) const {
		return is_member(key.c_str(), key.end_c_str());
	}
#endif

	value::Members value::get_member_names() const
	{
		JSON_ASSERT_MESSAGE( type_ == null_value || type_ == object_value,
			"in json::value::get_member_names(), value must be object_value");

		if (type_ == null_value)
			return value::Members();

		Members members;
		members.reserve(value_.map_->size());
		object_values::const_iterator it = value_.map_->begin();
		object_values::const_iterator itEnd = value_.map_->end();
		for (; it != itEnd; ++it)
			members.push_back(std::string((*it).first.data(), (*it).first.length()));
		
		return members;
	}
	//
	//# ifdef JSON_USE_CPPTL
	// EnumMemberNames
	// value::enumMemberNames() const
	//{
	//   if ( type_ == object_value )
	//   {
	//      return CppTL::Enum::any(  CppTL::Enum::transform(
	//         CppTL::Enum::keys( *(value_.map_), CppTL::Type<const czstring &>() ),
	//         MemberNamesTransform() ) );
	//   }
	//   return EnumMemberNames();
	//}
	//
	//
	// EnumValues
	// value::enumValues() const
	//{
	//   if ( type_ == object_value  ||  type_ == array_value )
	//      return CppTL::Enum::anyValues( *(value_.map_),
	//                                     CppTL::Type<const value &>() );
	//   return EnumValues();
	//}
	//
	//# endif

	static bool IsIntegral(double d)
	{
		double integral_part;
		return modf(d, &integral_part) == 0.0;
	}

	bool value::is_null() const { return type_ == null_value; }

	bool value::is_bool() const { return type_ == boolean_value; }

	bool value::is_int() const
	{
		switch (type_)
		{
		case int_value:
			return value_.int_ >= min_int && value_.int_ <= max_int;
		case uint_value:
			return value_.uint_ <= uint(max_int);
		case real_value:
			return value_.real_ >= min_int && value_.real_ <= max_int &&
				IsIntegral(value_.real_);
		default:
			break;
		}
		return false;
	}

	bool value::is_uint() const
	{
		switch (type_) {
		case int_value:
			return value_.int_ >= 0 && largest_uint(value_.int_) <= largest_uint(maxUInt);
		case uint_value:
			return value_.uint_ <= maxUInt;
		case real_value:
			return value_.real_ >= 0 && value_.real_ <= maxUInt &&
				IsIntegral(value_.real_);
		default:
			break;
		}
		return false;
	}

	bool value::is_int64() const
	{
#if defined(JSON_HAS_INT64)
		switch (type_)
		{
		case int_value:
			return true;
		case uint_value:
			return value_.uint_ <= uint64(max_int64);
		case real_value:
			// Note that max_int64 (= 2^63 - 1) is not exactly representable as a
			// double, so double(max_int64) will be rounded up to 2^63. Therefore we
			// require the value to be strictly less than the limit.
			return value_.real_ >= double(min_int64) &&
				value_.real_ < double(max_int64) && IsIntegral(value_.real_);
		default:
			break;
		}
#endif // JSON_HAS_INT64
		return false;
	}

	bool value::is_uint64() const
	{
#if defined(JSON_HAS_INT64)
		switch (type_)
		{
		case int_value:
			return value_.int_ >= 0;
		case uint_value:
			return true;
		case real_value:
			// Note that max_uint64 (= 2^64 - 1) is not exactly representable as a
			// double, so double(max_uint64) will be rounded up to 2^64. Therefore we
			// require the value to be strictly less than the limit.
			return value_.real_ >= 0 && value_.real_ < max_uint64_as_double &&
				IsIntegral(value_.real_);
		default:
			break;
		}
#endif // JSON_HAS_INT64
		return false;
	}

	bool value::is_integral() const
	{
#if defined(JSON_HAS_INT64)
		return is_int64() || is_uint64();
#else
		return is_int() || is_uint();
#endif
	}

	bool value::is_double() const { return type_ == real_value || is_integral(); }

	bool value::is_numeric() const { return is_integral() || is_double(); }

	bool value::is_string() const { return type_ == string_value; }

	bool value::is_array() const { return type_ == array_value; }

	bool value::is_object() const { return type_ == object_value; }

	void value::set_comment(const char* comment, size_t len, comment_placement placement)
	{
		if (!comments_)
			comments_ = new comment_info[number_of_comment_placement];
		if ((len > 0) && (comment[len - 1] == '\n'))
			// Always discard trailing newline, to aid indentation.
			len -= 1;
		
		comments_[placement].set_comment(comment, len);
	}

	void value::set_comment(const char* comment, comment_placement placement)
	{
		set_comment(comment, strlen(comment), placement);
	}

	void value::set_comment(const std::string& comment, comment_placement placement)
	{
		set_comment(comment.c_str(), comment.length(), placement);
	}

	bool value::has_comment(comment_placement placement) const
	{
		return comments_ != 0 && comments_[placement].comment_ != 0;
	}

	std::string value::get_comment(comment_placement placement) const
	{
		if (has_comment(placement))
			return comments_[placement].comment_;
		return "";
	}

	void value::set_offset_start(size_t start) { start_ = start; }

	void value::set_offset_limit(size_t limit) { limit_ = limit; }

	size_t value::get_offset_start() const { return start_; }

	size_t value::get_offset_limit() const { return limit_; }

	std::string value::to_styled_string() const
	{
		styled_writer writer;
		return writer.write(*this);
	}

	value::const_iterator value::begin() const
	{
		switch (type_)
		{
		case array_value:
		case object_value:
			if (value_.map_)
				return const_iterator(value_.map_->begin());
			break;
		default:
			break;
		}
		return const_iterator();
	}

	value::const_iterator value::end() const
	{
		switch (type_)
		{
		case array_value:
		case object_value:
			if (value_.map_)
				return const_iterator(value_.map_->end());
			break;
		default:
			break;
		}
		return const_iterator();
	}

	value::iterator value::begin()
	{
		switch (type_)
		{
		case array_value:
		case object_value:
			if (value_.map_)
				return iterator(value_.map_->begin());
			break;
		default:
			break;
		}
		return iterator();
	}

	value::iterator value::end()
	{
		switch (type_)
		{
		case array_value:
		case object_value:
			if (value_.map_)
				return iterator(value_.map_->end());
			break;
		default:
			break;
		}
		return iterator();
	}

	// class path_argument
	// //////////////////////////////////////////////////////////////////

	path_argument::path_argument()
		: key_(), index_(), kind_(kind_none)
	{ }

	path_argument::path_argument(array_index index)
		: key_(), index_(index), kind_(kind_index)
	{ }

	path_argument::path_argument(const char* key)
		: key_(key), index_(), kind_(kind_key)
	{ }

	path_argument::path_argument(const std::string& key)
		: key_(key.c_str()), index_(), kind_(kind_key)
	{ }

	// class path
	// //////////////////////////////////////////////////////////////////

	path::path(const std::string& path,
		const path_argument& a1,
		const path_argument& a2,
		const path_argument& a3,
		const path_argument& a4,
		const path_argument& a5)
	{
		InArgs in;
		in.push_back(&a1);
		in.push_back(&a2);
		in.push_back(&a3);
		in.push_back(&a4);
		in.push_back(&a5);
		makePath(path, in);
	}

	void path::makePath(const std::string& path, const InArgs& in)
	{
		const char* current = path.c_str();
		const char* end = current + path.length();
		InArgs::const_iterator itInArg = in.begin();
		while (current != end)
		{
			if (*current == '[')
			{
				++current;
				if (*current == '%')
					add_path_in_arg(path, in, itInArg, path_argument::kind_index);
				else
				{
					array_index index = 0;
					for (; current != end && *current >= '0' && *current <= '9'; ++current)
						index = index * 10 + array_index(*current - '0');
					args_.push_back(index);
				}
				if (current == end || *current++ != ']')
					invalidPath(path, int(current - path.c_str()));
			}
			else if (*current == '%')
			{
				add_path_in_arg(path, in, itInArg, path_argument::kind_key);
				++current;
			}
			else if (*current == '.') {
				++current;
			}
			else
			{
				const char* beginName = current;
				while (current != end && !strchr("[.", *current))
					++current;
				args_.push_back(std::string(beginName, current));
			}
		}
	}

	void path::add_path_in_arg(const std::string& /*path*/, const InArgs& in, InArgs::const_iterator& itInArg, path_argument::kind kind)
	{
		if (itInArg == in.end())
		{
			// Error: missing argument %d
		}
		else if ((*itInArg)->kind_ != kind)
		{
			// Error: bad argument type
		}
		else
		{
			args_.push_back(**itInArg);
		}
	}

	void path::invalidPath(const std::string& /*path*/, int /*location*/)
	{
		// Error: invalid path.
	}

	const value& path::resolve(const value& root) const
	{
		const value* node = &root;
		for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it)
		{
			const path_argument& arg = *it;
			if (arg.kind_ == path_argument::kind_index)
			{
				if (!node->is_array() || !node->is_valid_index(arg.index_))
				{
					// Error: unable to resolve path (array value expected at position...
				}
				node = &((*node)[arg.index_]);
			}
			else if (arg.kind_ == path_argument::kind_key)
			{
				if (!node->is_object())
				{
					// Error: unable to resolve path (object value expected at position...)
				}
				node = &((*node)[arg.key_]);
				if (node == &value::nullRef)
				{
					// Error: unable to resolve path (object has no member named '' at
					// position...)
				}
			}
		}
		return *node;
	}

	value path::resolve(const value& root, const value& defaultValue) const
	{
		const value* node = &root;
		for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it)
		{
			const path_argument& arg = *it;
			if (arg.kind_ == path_argument::kind_index)
			{
				if (!node->is_array() || !node->is_valid_index(arg.index_))
					return defaultValue;
				node = &((*node)[arg.index_]);
			}
			else if (arg.kind_ == path_argument::kind_key)
			{
				if (!node->is_object())
					return defaultValue;
				node = &((*node)[arg.key_]);
				if (node == &value::nullRef)
					return defaultValue;
			}
		}
		return *node;
	}

	value& path::make(value& root) const
	{
		value* node = &root;
		for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it)
		{
			const path_argument& arg = *it;
			if (arg.kind_ == path_argument::kind_index)
			{
				if (!node->is_array())
				{
					// Error: node is not an array at position ...
				}
				node = &((*node)[arg.index_]);
			}
			else if (arg.kind_ == path_argument::kind_key)
			{
				if (!node->is_object())
				{
					// Error: node is not an object at position...
				}
				node = &((*node)[arg.key_]);
			}
		}
		return *node;
	}

} // namespace json
