
#include "Environment.h"
#include "SValue.h"

SValueRef Environment::get( const Symbol& sym ) const
{
  auto value = std::make_shared< SValue >();
  auto it = env.find( sym );
  REQUIRE( value, it != env.end(), sym.label + " not found" );
  *value = *it->second; // Copy
  return value;
}

void Environment::set( const Symbol& sym, SValueRef v )
{
  env[ sym ] = std::make_shared< SValue >( *v );
}
