
#include "Environment.h"
#include "SValue.h"

SValueRef getFromEnv( const std::string& sym, Environment& e, SValueRef v )
{
  auto it = e.find( sym );
  REQUIRE( v, it != e.end(), sym + " not found" );
  // Make an SValue copy.
  return std::make_shared< SValue >( *it->second );
}

void addToEnv( Environment& e, const std::string& sym, SValueRef v )
{
  e[ sym ] = std::make_shared< SValue >( *v );
}
