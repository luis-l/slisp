
#pragma once

#include <string>
#include <unordered_map>

#include "SValueRef.h"

using Environment = std::unordered_map< std::string, SValueRef >;

// Get a copy for the symbol from the environment.
SValueRef getFromEnv( const std::string& sym, Environment& e, SValueRef v );
