
#include "SValue.h"
#include <ostream>

std::ostream& operator<<( std::ostream& o, const Sexpr& t )
{
  return o << "sexpr";
}

std::ostream& operator<<( std::ostream& o, const QExpr& t )
{
  return o << "qexpr";
}

std::ostream& operator<<( std::ostream& o, const Error& e )
{
  return o << e.message;
}

std::unordered_map< const SValue*, std::size_t > getDepths( const SValue& r )
{
  std::unordered_map< const SValue*, std::size_t > depths;
  traverseLevelOrder( r, [ &depths ]( const SValue& v, std::size_t level ) { depths[ &v ] = level; } );
  return depths;
}

std::ostream& operator<<( std::ostream& o, const SValue& r )
{
  const std::unordered_map< const SValue*, std::size_t > depths = getDepths( r );

  traversePreorder( r, [ &depths, &o ]( const SValue& v ) {
    const std::string padding( depths.at( &v ) * 2, ' ' );
    o << padding << "value: '" << v.value << "'\n";
  } );

  return o;
}
