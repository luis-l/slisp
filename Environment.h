
#pragma once

#include "SValueRef.h"
#include "Symbol.h"

#include <unordered_map>

class Environment
{
public:
  Environment( Environment* parent = nullptr );

  SValueRef get( const Symbol& s ) const;
  void set( const Symbol& s, SValueRef );

  Environment* parent = nullptr;

private:
  std::unordered_map< Symbol, SValueRef, SymbolHash > env;
};
