// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef JSON_FORWARDS_H_INCLUDED
#define JSON_FORWARDS_H_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
#include "config.h"
#endif // if !defined(JSON_IS_AMALGAMATION)

namespace json
{
	// writer.h
	class fast_writer;
	class styled_writer;

	// reader.h
	class reader;

	// features.h
	class features;

	// value.h
	typedef unsigned int array_index;
	class static_string;
	class path;
	class path_argument;
	class value;
	class value_iterator_base;
	class value_iterator;
	class value_const_iterator;
} // namespace json

#endif // JSON_FORWARDS_H_INCLUDED
