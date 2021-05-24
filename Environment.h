
#pragma once

#include <string>
#include <unordered_map>

#include "SValueRef.h"

using Environment = std::unordered_map< std::string, SValueRef >;

// Get a copy for the symbol from the environment.
SValueRef getFromEnv( const std::string& sym, Environment& e, SValueRef v );

/// @brief Adds a copy of the s-expression to the environment with the given symbol.
void addToEnv( Environment& e, const std::string& sym, SValueRef v );
