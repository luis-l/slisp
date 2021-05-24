
#pragma once

#include "SValueRef.h"
#include "Symbol.h"

#include <unordered_map>

class Environment
{
public:
  SValueRef get( const Symbol& s ) const;
  void set( const Symbol& s, SValueRef );

private:
  std::unordered_map< Symbol, SValueRef, SymbolHash > env;
};
