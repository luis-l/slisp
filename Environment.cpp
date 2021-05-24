
#include "Environment.h"
#include "SValue.h"

Environment::Environment( Environment* parent ) : parent( parent )
{}

SValueRef Environment::get( const Symbol& sym ) const
{
  auto value = std::make_shared< SValue >();
  auto it = env.find( sym );

  //  Found in local environment.
  if ( it != env.end() )
  {
    *value = *it->second; // Copy
    return value;
  }

  // Try parent.
  REQUIRE( value, parent != nullptr, sym.label + " not found" );

  return parent->get( sym );
}

void Environment::set( const Symbol& sym, SValueRef v )
{
  env[ sym ] = std::make_shared< SValue >( *v );
}

void Environment::rootSet( const Symbol& s, SValueRef v )
{
  // Find root. An environment with no parent.
  Environment* root = this;
  while ( root->parent )
  {
    root = root->parent;
  }

  root->set( s, v );
}
