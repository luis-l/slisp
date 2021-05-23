
#include "Environment.h"
#include "SValue.h"

SValueRef getSymbol( const std::string& sym, Environment& e, SValueRef v )
{
  auto it = e.find( sym );
  REQUIRE( v, it != e.end(), sym + " not found" );
  return it->second;
}
