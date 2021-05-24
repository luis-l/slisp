
#pragma once

#include "SValueRef.h"
#include "Symbol.h"

#include <unordered_map>

using Environment = std::unordered_map< Symbol, SValueRef, SymbolHash >;

// Get a copy for the symbol from the environment.
SValueRef getFromEnv( const Symbol& sym, Environment& e, SValueRef v );

/// @brief Adds a copy of the s-expression to the environment with the given symbol.
void addToEnv( Environment& e, const Symbol& sym, SValueRef v );
