// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

// included by json_value.cpp

namespace json {

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class value_iterator_base
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	value_iterator_base::value_iterator_base()
		: current_(), isNull_(true)
	{ }

	value_iterator_base::value_iterator_base(
		const value::object_values::iterator & current)
		: current_(current), isNull_(false)
	{ }

	value& value_iterator_base::deref() const
	{
		return current_->second;
	}

	void value_iterator_base::increment()
	{
		++current_;
	}

	void value_iterator_base::decrement()
	{
		--current_;
	}

	value_iterator_base::difference_type
		value_iterator_base::compute_distance(const self_type & other) const
	{
#ifdef JSON_USE_CPPTL_SMALLMAP
		return other.current_ - current_;
#else
		// Iterator for null value are initialized using the default
		// constructor, which initialize current_ to the default
		// std::map::iterator. As begin() and end() are two instance
		// of the default std::map::iterator, they can not be compared.
		// To allow this, we handle this comparison specifically.
		if (isNull_ && other.isNull_)
			return 0;

		// Usage of std::distance is not portable (does not compile with Sun Studio 12
		// RogueWave STL,
		// which is the one used by default).
		// Using a portable hand-made version for non random iterator instead:
		//   return difference_type( std::distance( current_, other.current_ ) );
		difference_type myDistance = 0;
		for (value::object_values::iterator it = current_; it != other.current_; ++it)
			++myDistance;
		
		return myDistance;
#endif
	}

	bool value_iterator_base::is_equal(const self_type& other) const
	{
		if (isNull_)
			return other.isNull_;
		
		return current_ == other.current_;
	}

	void value_iterator_base::copy(const self_type& other)
	{
		current_ = other.current_;
		isNull_ = other.isNull_;
	}

	value value_iterator_base::key() const {
		const value::czstring czstring = (*current_).first;

		if (czstring.data())
		{
			if (czstring.isStaticString())
				return value(static_string(czstring.data()));

			return value(czstring.data(), czstring.data() + czstring.length());
		}

		return value(czstring.index());
	}

	uint value_iterator_base::index() const
	{
		const value::czstring czstring = (*current_).first;

		if (!czstring.data())
			return (czstring.index());

		return (value::uint(-1));
	}

	std::string value_iterator_base::name() const {
		char const* keey;
		char const* end;

		keey = member_name(&end);

		if (!keey)
			return (std::string());

		return (std::string(keey, end));
	}

	char const* value_iterator_base::member_name() const
	{
		const char* cname = (*current_).first.data();
		return (cname ? cname : "");
	}

	char const* value_iterator_base::member_name(char const** end) const
	{
		const char* cname = (*current_).first.data();
		if (!cname)
		{
			*end = nullptr;
			return (nullptr);
		}

		*end = cname + (*current_).first.length();
		return (cname);
	}

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class value_const_iterator
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	value_const_iterator::value_const_iterator()
	{ }

	value_const_iterator::value_const_iterator(const value::object_values::iterator & current)
		: value_iterator_base(current)
	{ }

	value_const_iterator & value_const_iterator::operator=(const value_iterator_base & other)
	{
		copy(other);
		return (*this);
	}

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class value_iterator
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	value_iterator::value_iterator()
	{ }

	value_iterator::value_iterator(const value::object_values::iterator & current)
		: value_iterator_base(current)
	{ }

	value_iterator::value_iterator(const value_const_iterator & other)
		: value_iterator_base(other)
	{ }

	value_iterator::value_iterator(const value_iterator & other)
		: value_iterator_base(other)
	{ }

	value_iterator & value_iterator::operator =(const self_type& other)
	{
		copy(other);
		return (*this);
	}

} // namespace json
