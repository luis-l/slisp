#pragma once

#include <memory>

class SValue;

/// @brief Parses a string iterable.
/// @tparam IteratorT Iterable type for strings. e.g. std::string::iterator
/// @param begin Where to start parsing.
/// @param end Where to end parsing.
/// @return The S-expression for the parsed contents.
template < typename IteratorT >
std::shared_ptr< SValue > parse( IteratorT begin, IteratorT end );
