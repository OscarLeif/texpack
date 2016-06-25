// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef CPPTL_JSON_H_INCLUDED
#define CPPTL_JSON_H_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
#include "forwards.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <string>
#include <vector>
#include <exception>

#ifndef JSON_USE_CPPTL_SMALLMAP
#include <map>
#else
#include <cpptl/smallmap.h>
#endif
#ifdef JSON_USE_CPPTL
#include <cpptl/forwards.h>
#endif

// Disable warning C4251: <data member>: <type> needs to have dll-interface to
// be used by...
#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

/** \brief JSON (JavaScript Object Notation).
 */
namespace json
{
	/** Base class for all exceptions we throw.
	 *
	 * We use nothing but these internally. Of course, STL can throw others.
	 */
	class JSON_API exception : public std::exception
	{
	protected:
		std::string msg_;

	public:
		exception(std::string const& msg);
		~exception() throw() override;
		char const* what() const throw() override;
	};

	/** Exceptions which the user cannot easily avoid.
	 *
	 * E.g. out-of-memory (when we use malloc), stack-overflow, malicious input
	 *
	 * \remark derived from json::exception
	 */
	class JSON_API runtime_error : public exception
	{
	public:
		runtime_error(std::string const& msg);
	};

	/** Exceptions thrown by JSON_ASSERT/JSON_FAIL macros.
	 *
	 * These are precondition-violations (user bugs) and internal errors (our bugs).
	 *
	 * \remark derived from json::exception
	 */
	class JSON_API logic_error : public exception
	{
	public:
		logic_error(std::string const& msg);
	};

	/// used internally
	void throw_runtime_error(std::string const& msg);
	/// used internally
	void throw_logic_error(std::string const& msg);

	/** \brief Type of the value held by a value object.
	 */
	enum value_type
	{
		null_value = 0, ///< 'null' value
		int_value,      ///< signed integer value
		uint_value,     ///< unsigned integer value
		real_value,     ///< double value
		string_value,   ///< UTF-8 string value
		boolean_value,  ///< bool value
		array_value,    ///< array value (ordered list)
		object_value    ///< object value (collection of name/value pairs).
	};

	enum comment_placement
	{
		comment_before = 0,      ///< a comment placed on the line before a value
		comment_after_on_same_line, ///< a comment just after a value on the same line
		comment_after, ///< a comment on the line after a value (only make sense for
		/// root value)
		number_of_comment_placement
	};

	//# ifdef JSON_USE_CPPTL
	//   typedef CppTL::AnyEnumerator<const char *> EnumMemberNames;
	//   typedef CppTL::AnyEnumerator<const value &> EnumValues;
	//# endif

	/** \brief Lightweight wrapper to tag static string.
	 *
	 * value constructor and object_value member assignement takes advantage of the
	 * static_string and avoid the cost of string duplication when storing the
	 * string or the member name.
	 *
	 * Example of usage:
	 * \code
	 * json::value aValue( static_string("some text") );
	 * json::value object;
	 * static const static_string code("code");
	 * object[code] = 1234;
	 * \endcode
	 */
	class JSON_API static_string
	{
	public:
		explicit static_string(const char* czstring)
			: c_str_(czstring)
		{ }

		operator const char*() const
		{ return c_str_; }

		const char* c_str() const
		{ return c_str_; }

	private:
		const char* c_str_;
	};

	/** \brief Represents a <a HREF="http://www.json.org">JSON</a> value.
	 *
	 * This class is a discriminated union wrapper that can represents a:
	 * - signed integer [range: value::min_int - value::max_int]
	 * - unsigned integer (range: 0 - value::maxUInt)
	 * - double
	 * - UTF-8 string
	 * - boolean
	 * - 'null'
	 * - an ordered list of value
	 * - collection of name/value pairs (javascript object)
	 *
	 * The type of the held value is represented by a #value_type and
	 * can be obtained using type().
	 *
	 * Values of an #object_value or #array_value can be accessed using operator[]()
	 * methods.
	 * Non-const methods will automatically create the a #null_value element
	 * if it does not exist.
	 * The sequence of an #array_value will be automatically resized and initialized
	 * with #null_value. resize() can be used to enlarge or truncate an #array_value.
	 *
	 * The get() methods can be used to obtain default value in the case the
	 * required element does not exist.
	 *
	 * It is possible to iterate over the list of a #object_value values using
	 * the get_member_names() method.
	 *
	 * \note #value string-length fit in size_t, but keys must be < 2^30.
	 * (The reason is an implementation detail.) A #char_reader will raise an
	 * exception if a bound is exceeded to avoid security holes in your app,
	 * but the value API does *not* check bounds. That is the responsibility
	 * of the caller.
	 */
	class JSON_API value
	{
		friend class value_iterator_base;
	public:
		typedef std::vector<std::string> Members;
		typedef value_iterator iterator;
		typedef value_const_iterator const_iterator;
		typedef json::uint uint;
#if defined(JSON_HAS_INT64)
		typedef json::uint64 uint64;
		typedef json::int64 int64;
#endif // defined(JSON_HAS_INT64)
		typedef json::largest_int largest_int;
		typedef json::largest_uint largest_uint;
		typedef json::array_index array_index;

		static const value & null;  ///< We regret this reference to a global instance; prefer the simpler value().
		static const value & nullRef;  ///< just a kludge for binary-compatibility; same as null
		/// Minimum signed integer value that can be stored in a json::value.
		static const largest_int min_largest_int;
		/// Maximum signed integer value that can be stored in a json::value.
		static const largest_int max_largest_int;
		/// Maximum unsigned integer value that can be stored in a json::value.
		static const largest_uint max_largest_uint;

		/// Minimum signed int value that can be stored in a json::value.
		static const int min_int;
		/// Maximum signed int value that can be stored in a json::value.
		static const int max_int;
		/// Maximum unsigned int value that can be stored in a json::value.
		static const uint maxUInt;

#if defined(JSON_HAS_INT64)
		/// Minimum signed 64 bits int value that can be stored in a json::value.
		static const int64 min_int64;
		/// Maximum signed 64 bits int value that can be stored in a json::value.
		static const int64 max_int64;
		/// Maximum unsigned 64 bits int value that can be stored in a json::value.
		static const uint64 max_uint64;
#endif // defined(JSON_HAS_INT64)

	private:
#ifndef JSONCPP_DOC_EXCLUDE_IMPLEMENTATION
		class czstring
		{
		public:
			enum duplication_policy
			{
				no_duplication = 0,
				duplicate,
				duplicate_on_copy
			};

			czstring(array_index index);
			czstring(char const * str, unsigned length, duplication_policy allocate);
			czstring(czstring const& other);
			~czstring();
			czstring& operator=(czstring other);

			bool operator<(czstring const& other) const;
			bool operator==(czstring const& other) const;

			array_index index() const;
			//const char* c_str() const; ///< \deprecated
			char const * data() const;
			unsigned length() const;
			bool isStaticString() const;

		private:
			void swap(czstring& other);

			struct string_storage
			{
				unsigned policy_ : 2;
				unsigned length_ : 30; // 1GB max
			};

			char const* cstr_;  // actually, a prefixed string, unless policy is noDup
			union
			{
				array_index index_;
				string_storage storage_;
			};
		};

	public:
#ifndef JSON_USE_CPPTL_SMALLMAP
		typedef std::map<czstring, value> object_values;
#else
		typedef CppTL::SmallMap<czstring, value> object_values;
#endif // ifndef JSON_USE_CPPTL_SMALLMAP
#endif // ifndef JSONCPP_DOC_EXCLUDE_IMPLEMENTATION

	public:
		/** \brief Create a default value of the given type.

		  This is a very useful constructor.
		  To create an empty array, pass array_value.
		  To create an empty object, pass object_value.
		  Another value can then be set to this one by assignment.
		  This is useful since clear() and resize() will not alter types.

		  Examples:
		  \code
		  json::value null_value; // null
		  json::value arr_value(json::array_value); // []
		  json::value obj_value(json::object_value); // {}
		  \endcode
		  */
		value(value_type type = null_value);
		value(int value);
		value(uint value);
#if defined(JSON_HAS_INT64)
		value(int64 value);
		value(uint64 value);
#endif // if defined(JSON_HAS_INT64)
		value(double value);
		value(const char* value); ///< Copy til first 0. (NULL causes to seg-fault.)
		value(const char* begin, const char* end); ///< Copy all, incl zeroes.
		/** \brief Constructs a value from a static string.

		 * Like other value string constructor but do not duplicate the string for
		 * internal storage. The given string must remain alive after the call to this
		 * constructor.
		 * \note This works only for null-terminated strings. (We cannot change the
		 *   size of this class, so we have nowhere to store the length,
		 *   which might be computed later for various operations.)
		 *
		 * Example of usage:
		 * \code
		 * static static_string foo("some text");
		 * json::value aValue(foo);
		 * \endcode
		 */
		value(const static_string& value);
		value(const std::string& value); ///< Copy data() til size(). Embedded zeroes too.
#ifdef JSON_USE_CPPTL
		value(const CppTL::ConstString& value);
#endif
		value(bool value);
		/// Deep copy.
		value(const value& other);
		~value();

		/// Deep copy, then swap(other).
		/// \note Over-write existing comments. To preserve comments, use #swap_payload().
		value& operator=(value other);
		/// Swap everything.
		void swap(value& other);
		/// Swap values but leave comments and source offsets in place.
		void swap_payload(value& other);

		value_type type() const;

		/// Compare payload only, not comments etc.
		bool operator<(const value& other) const;
		bool operator<=(const value& other) const;
		bool operator>=(const value& other) const;
		bool operator>(const value& other) const;
		bool operator==(const value& other) const;
		bool operator!=(const value& other) const;
		int compare(const value& other) const;

		const char* as_cstring() const; ///< Embedded zeroes could cause you trouble!
		std::string as_string() const; ///< Embedded zeroes are possible.
		/** Get raw char* of string-value.
		 *  \return false if !string. (Seg-fault if str or end are NULL.)
		 */
		bool get_string(char const** begin, char const** end) const;
#ifdef JSON_USE_CPPTL
		CppTL::ConstString asConstString() const;
#endif
		int as_int() const;
		uint as_uint() const;
#if defined(JSON_HAS_INT64)
		int64 as_int64() const;
		uint64 as_uint64() const;
#endif // if defined(JSON_HAS_INT64)
		largest_int as_largest_int() const;
		largest_uint as_largest_uint() const;
		float as_float() const;
		double as_double() const;
		bool as_bool() const;

		bool is_null() const;
		bool is_bool() const;
		bool is_int() const;
		bool is_int64() const;
		bool is_uint() const;
		bool is_uint64() const;
		bool is_integral() const;
		bool is_double() const;
		bool is_numeric() const;
		bool is_string() const;
		bool is_array() const;
		bool is_object() const;

		bool is_convertible_to(value_type other) const;

		/// Number of values in array or object
		array_index size() const;

		/// \brief Return true if empty array, empty object, or null;
		/// otherwise, false.
		bool empty() const;

		/// Return is_null()
		bool operator!() const;

		/// Remove all object members and array elements.
		/// \pre type() is array_value, object_value, or null_value
		/// \post type() is unchanged
		void clear();

		/// Resize the array to size elements.
		/// New elements are initialized to null.
		/// May only be called on null_value or array_value.
		/// \pre type() is array_value or null_value
		/// \post type() is array_value
		void resize(array_index size);

		/// Access an array element (zero based index ).
		/// If the array contains less than index element, then null value are
		/// inserted
		/// in the array so that its size is index+1.
		/// (You may need to say 'value[0u]' to get your compiler to distinguish
		///  this from the operator[] which takes a string.)
		value& operator[](array_index index);

		/// Access an array element (zero based index ).
		/// If the array contains less than index element, then null value are
		/// inserted
		/// in the array so that its size is index+1.
		/// (You may need to say 'value[0u]' to get your compiler to distinguish
		///  this from the operator[] which takes a string.)
		value& operator[](int index);

		/// Access an array element (zero based index )
		/// (You may need to say 'value[0u]' to get your compiler to distinguish
		///  this from the operator[] which takes a string.)
		const value& operator[](array_index index) const;

		/// Access an array element (zero based index )
		/// (You may need to say 'value[0u]' to get your compiler to distinguish
		///  this from the operator[] which takes a string.)
		const value& operator[](int index) const;

		/// If the array contains at least index+1 elements, returns the element
		/// value,
		/// otherwise returns defaultValue.
		value get(array_index index, const value& defaultValue) const;
		/// Return true if index < size().
		bool is_valid_index(array_index index) const;
		/// \brief Append value to array at the end.
		///
		/// Equivalent to jsonvalue[jsonvalue.size()] = value;
		value& append(const value& value);

		/// Access an object value by name, create a null member if it does not exist.
		/// \note Because of our implementation, keys are limited to 2^30 -1 chars.
		///  Exceeding that will cause an exception.
		value& operator[](const char* key);
		/// Access an object value by name, returns null if there is no member with
		/// that name.
		const value& operator[](const char* key) const;
		/// Access an object value by name, create a null member if it does not exist.
		/// \param key may contain embedded nulls.
		value& operator[](const std::string& key);
		/// Access an object value by name, returns null if there is no member with
		/// that name.
		/// \param key may contain embedded nulls.
		const value& operator[](const std::string& key) const;
		/** \brief Access an object value by name, create a null member if it does not
		 exist.

		 * If the object has no entry for that name, then the member name used to store
		 * the new entry is not duplicated.
		 * Example of use:
		 * \code
		 * json::value object;
		 * static const static_string code("code");
		 * object[code] = 1234;
		 * \endcode
		 */
		value& operator[](const static_string& key);
#ifdef JSON_USE_CPPTL
		/// Access an object value by name, create a null member if it does not exist.
		value& operator[](const CppTL::ConstString& key);
		/// Access an object value by name, returns null if there is no member with
		/// that name.
		const value& operator[](const CppTL::ConstString& key) const;
#endif
		/// Return the member named key if it exist, defaultValue otherwise.
		/// \note deep copy
		value get(const char* key, const value& defaultValue) const;
		/// Return the member named key if it exist, defaultValue otherwise.
		/// \note deep copy
		/// \note key may contain embedded nulls.
		value get(const char* begin, const char* end, const value& defaultValue) const;
		/// Return the member named key if it exist, defaultValue otherwise.
		/// \note deep copy
		/// \param key may contain embedded nulls.
		value get(const std::string& key, const value& defaultValue) const;
#ifdef JSON_USE_CPPTL
		/// Return the member named key if it exist, defaultValue otherwise.
		/// \note deep copy
		value get(const CppTL::ConstString& key, const value& defaultValue) const;
#endif
		/// Most general and efficient version of is_member()const, get()const,
		/// and operator[]const
		/// \note As stated elsewhere, behavior is undefined if (end-begin) >= 2^30
		value const* find(char const* begin, char const* end) const;
		/// Most general and efficient version of object-mutators.
		/// \note As stated elsewhere, behavior is undefined if (end-begin) >= 2^30
		/// \return non-zero, but JSON_ASSERT if this is neither object nor null_value.
		value const* demand(char const* begin, char const* end);
		/// \brief Remove and return the named member.
		///
		/// Do nothing if it did not exist.
		/// \return the removed value, or null.
		/// \pre type() is object_value or null_value
		/// \post type() is unchanged
		/// \deprecated
		value remove_member(const char* key);
		/// Same as remove_member(const char*)
		/// \param key may contain embedded nulls.
		/// \deprecated
		value remove_member(const std::string& key);
		/// Same as remove_member(const char* begin, const char* end, value* removed),
		/// but 'key' is null-terminated.
		bool remove_member(const char* key, value* removed);
		/** \brief Remove the named map member.

			Update 'removed' iff removed.
			\param key may contain embedded nulls.
			\return true iff removed (no exceptions)
			*/
		bool remove_member(std::string const& key, value* removed);
		/// Same as remove_member(std::string const& key, value* removed)
		bool remove_member(const char* begin, const char* end, value* removed);
		/** \brief Remove the indexed array element.

			O(n) expensive operations.
			Update 'removed' iff removed.
			\return true iff removed (no exceptions)
			*/
		bool remove_index(array_index i, value* removed);

		/// Return true if the object has a member named key.
		/// \note 'key' must be null-terminated.
		bool is_member(const char* key) const;
		/// Return true if the object has a member named key.
		/// \param key may contain embedded nulls.
		bool is_member(const std::string& key) const;
		/// Same as is_member(std::string const& key)const
		bool is_member(const char* begin, const char* end) const;
#ifdef JSON_USE_CPPTL
		/// Return true if the object has a member named key.
		bool is_member(const CppTL::ConstString& key) const;
#endif

		/// \brief Return a list of the member names.
		///
		/// If null, return an empty list.
		/// \pre type() is object_value or null_value
		/// \post if type() was null_value, it remains null_value
		Members get_member_names() const;

		//# ifdef JSON_USE_CPPTL
		//      EnumMemberNames enumMemberNames() const;
		//      EnumValues enumValues() const;
		//# endif

		/// \deprecated Always pass len.
		JSONCPP_DEPRECATED("Use set_comment(std::string const&) instead.")
		void set_comment(const char* comment, comment_placement placement);
		/// Comments must be //... or /* ... */
		void set_comment(const char* comment, size_t len, comment_placement placement);
		/// Comments must be //... or /* ... */
		void set_comment(const std::string& comment, comment_placement placement);
		bool has_comment(comment_placement placement) const;
		/// Include delimiters and embedded newlines.
		std::string get_comment(comment_placement placement) const;

		std::string to_styled_string() const;

		const_iterator begin() const;
		const_iterator end() const;

		iterator begin();
		iterator end();

		// Accessors for the [start, limit) range of bytes within the JSON text from
		// which this value was parsed, if any.
		void set_offset_start(size_t start);
		void set_offset_limit(size_t limit);
		size_t get_offset_start() const;
		size_t get_offset_limit() const;

	private:
		void init_basic(value_type type, bool allocated = false);

		value& resolve_reference(const char* key);
		value& resolve_reference(const char* key, const char* end);

		struct comment_info
		{
			comment_info();
			~comment_info();

			void set_comment(const char* text, size_t len);

			char* comment_;
		};

		// struct MemberNamesTransform
		//{
		//   typedef const char *result_type;
		//   const char *operator()( const czstring &name ) const
		//   {
		//      return name.c_str();
		//   }
		//};

		union value_holder
		{
			largest_int int_;
			largest_uint uint_;
			double real_;
			bool bool_;
			char* string_;  // actually ptr to unsigned, followed by str, unless !allocated_
			object_values* map_;
		} value_;
		value_type type_ : 8;
		unsigned int allocated_ : 1; // Notes: if declared as bool, bitfield is useless.
		// If not allocated_, string_ must be null-terminated.
		comment_info* comments_;

		// [start, limit) byte offsets in the source JSON text from which this value
		// was extracted.
		size_t start_;
		size_t limit_;
	};

	/** \brief Experimental and untested: represents an element of the "path" to
	 * access a node.
	 */
	class JSON_API path_argument
	{
	public:
		friend class path;

		path_argument();
		path_argument(array_index index);
		path_argument(const char* key);
		path_argument(const std::string& key);

	private:
		enum kind
		{
			kind_none = 0,
			kind_index,
			kind_key
		};
		std::string key_;
		array_index index_;
		kind kind_;
	};

	/** \brief Experimental and untested: represents a "path" to access a node.
	 *
	 * Syntax:
	 * - "." => root node
	 * - ".[n]" => elements at index 'n' of root node (an array value)
	 * - ".name" => member named 'name' of root node (an object value)
	 * - ".name1.name2.name3"
	 * - ".[0][1][2].name1[3]"
	 * - ".%" => member name is provided as parameter
	 * - ".[%]" => index is provied as parameter
	 */
	class JSON_API path
	{
	public:
		path(const std::string& path,
			const path_argument& a1 = path_argument(),
			const path_argument& a2 = path_argument(),
			const path_argument& a3 = path_argument(),
			const path_argument& a4 = path_argument(),
			const path_argument& a5 = path_argument());

		const value& resolve(const value& root) const;
		value resolve(const value& root, const value& defaultValue) const;
		/// Creates the "path" to access the specified node and returns a reference on
		/// the node.
		value& make(value& root) const;

	private:
		typedef std::vector<const path_argument*> InArgs;
		typedef std::vector<path_argument> Args;

		void makePath(const std::string& path, const InArgs& in);
		void add_path_in_arg(const std::string& path, const InArgs& in, InArgs::const_iterator& itInArg, path_argument::kind kind);
		void invalidPath(const std::string& path, int location);

		Args args_;
	};

	/** \brief base class for value iterators.
	 *
	 */
	class JSON_API value_iterator_base
	{
	public:
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef unsigned int size_t;
		typedef int difference_type;
		typedef value_iterator_base self_type;

		bool operator==(const self_type& other) const { return is_equal(other); }

		bool operator!=(const self_type& other) const { return !is_equal(other); }

		difference_type operator-(const self_type& other) const {
			return other.compute_distance(*this);
		}

		/// Return either the index or the member name of the referenced value as a
		/// value.
		value key() const;

		/// Return the index of the referenced value, or -1 if it is not an array_value.
		uint index() const;

		/// Return the member name of the referenced value, or "" if it is not an
		/// object_value.
		/// \note Avoid `c_str()` on result, as embedded zeroes are possible.
		std::string name() const;

		/// Return the member name of the referenced value. "" if it is not an
		/// object_value.
		/// \deprecated This cannot be used for UTF-8 strings, since there can be embedded nulls.
		JSONCPP_DEPRECATED("Use `key = name();` instead.")
		char const* member_name() const;

		/// Return the member name of the referenced value, or NULL if it is not an
		/// object_value.
		/// \note Better version than member_name(). Allows embedded nulls.
		char const* member_name(char const** end) const;

	protected:
		value& deref() const;

		void increment();

		void decrement();

		difference_type compute_distance(const self_type& other) const;

		bool is_equal(const self_type& other) const;

		void copy(const self_type& other);

	private:
		value::object_values::iterator current_;
		// Indicates that iterator is for a null value.
		bool isNull_;

	public:
		// For some reason, BORLAND needs these at the end, rather
		// than earlier. No idea why.
		value_iterator_base();
		explicit value_iterator_base(const value::object_values::iterator& current);
	};

	/** \brief const iterator for object and array value.
	 *
	 */
	class JSON_API value_const_iterator : public value_iterator_base
	{
		friend class value;

	public:
		typedef const value value_type;
		//typedef unsigned int size_t;
		//typedef int difference_type;
		typedef const value& reference;
		typedef const value* pointer;
		typedef value_const_iterator self_type;

		value_const_iterator();

	private:
		/*! \internal Use by value to create an iterator.
		 */
		explicit value_const_iterator(const value::object_values::iterator& current);
	public:
		self_type& operator=(const value_iterator_base& other);

		self_type operator++(int)
		{
			self_type temp(*this);
			++*this;
			return temp;
		}

		self_type operator--(int)
		{
			self_type temp(*this);
			--*this;
			return temp;
		}

		self_type& operator--()
		{
			decrement();
			return *this;
		}

		self_type& operator++()
		{
			increment();
			return *this;
		}

		reference operator*() const { return deref(); }

		pointer operator->() const { return &deref(); }
	};

	/** \brief Iterator for object and array value.
	 */
	class JSON_API value_iterator : public value_iterator_base
	{
		friend class value;

	public:
		typedef value value_type;
		typedef unsigned int size_t;
		typedef int difference_type;
		typedef value& reference;
		typedef value* pointer;
		typedef value_iterator self_type;

		value_iterator();
		value_iterator(const value_const_iterator& other);
		value_iterator(const value_iterator& other);

	private:
		/*! \internal Use by value to create an iterator.
		 */
		explicit value_iterator(const value::object_values::iterator& current);
	public:
		self_type& operator=(const self_type& other);

		self_type operator++(int)
		{
			self_type temp(*this);
			++*this;
			return temp;
		}

		self_type operator--(int)
		{
			self_type temp(*this);
			--*this;
			return temp;
		}

		self_type& operator--()
		{
			decrement();
			return *this;
		}

		self_type& operator++()
		{
			increment();
			return *this;
		}

		reference operator*() const { return deref(); }

		pointer operator->() const { return &deref(); }
	};
} // namespace json

namespace std
{
	/// Specialize std::swap() for json::value.
	template<>
	inline void swap(json::value& a, json::value& b) { a.swap(b); }
}

#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(pop)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

#endif // CPPTL_JSON_H_INCLUDED
