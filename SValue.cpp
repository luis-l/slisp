
#include "SValue.h"
#include <ostream>

std::ostream& operator<<( std::ostream& o, const ExpressionType& t )
{
  return o << "expr";
}

std::ostream& operator<<( std::ostream& o, const Error& e )
{
  return o << e.message;
}

std::unordered_map< const SValue*, std::size_t > getDepths( const std::unique_ptr< SValue >& r )
{
  std::unordered_map< const SValue*, std::size_t > depths;
  r->traverseLevelOrder( [ &depths ]( const SValue& v, std::size_t level ) { depths[ &v ] = level; } );
  return depths;
}

std::ostream& operator<<( std::ostream& o, const std::unique_ptr< SValue >& r )
{
  const std::unordered_map< const SValue*, std::size_t > depths = getDepths( r );

  r->traversePreorder( [ &depths, &o ]( const SValue& v ) {
    const std::string padding( depths.at( &v ) * 2, ' ' );
    o << padding << "value: '" << v.value << "'\n";
  } );

  return o;
}
