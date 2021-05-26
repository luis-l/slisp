
#pragma once

#include "Symbol.h"

#include <unordered_map>

class SValue;

class Environment
{
public:
  Environment( Environment* parent = nullptr );

  /// Gets a copy of the value for the given symbol.
  // Copy is stored in v.
  SValue* get( const Symbol& s, SValue* v ) const;

  /// Define a symbol with a given value. A copy of the value is stored.
  void set( const Symbol& s, const SValue& v );

  /// Defines the symbol at the root. A copy of the value is stored.
  void rootSet( const Symbol& s, const SValue& v );

  Environment* parent = nullptr;

private:
  // If we want to use a map, then the SValues must be stored as shared_ptr. (or other indirection that supports copy).
  // This is because map can modify the internal buffer and do copies.
  //
  // If using a vector to store, we could use unique_ptr
  std::unordered_map< Symbol, std::shared_ptr< SValue >, SymbolHash > env;
};
