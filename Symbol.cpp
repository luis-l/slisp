
#include "Symbol.h"

bool Symbol::operator==( const Symbol& other ) const
{
  return label == other.label;
}

std::size_t SymbolHash::operator()( const Symbol& s ) const
{
  return std::hash< std::string >()( s.label );
}

std::ostream& operator<<( std::ostream& o, const Symbol& s )
{
  return o << s.label;
}
