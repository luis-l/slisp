
#pragma once

#include <string>
#include <unordered_map>

#include "SValueRef.h"

using Environment = std::unordered_map< std::string, SValueRef >;

SValueRef getSymbol( const std::string& sym, Environment& e, SValueRef v );
