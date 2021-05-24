#pragma once

#include <ostream>
#include <string>

struct Symbol
{
  std::string label;

  bool operator==( const Symbol& other ) const;
};

struct SymbolHash
{
  std::size_t operator()( const Symbol& k ) const;
};

std::ostream& operator<<( std::ostream& o, const Symbol& s );
