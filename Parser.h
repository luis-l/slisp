#pragma once

#include "SValueRef.h"

/// @brief Parses a string iterable.
/// @tparam IteratorT Iterable type for strings. e.g. std::string::iterator
/// @param begin Where to start parsing.
/// @param end Where to end parsing.
/// @return The S-expression for the parsed contents.
template < typename IteratorT >
SValueRef parse( IteratorT begin, IteratorT end );
