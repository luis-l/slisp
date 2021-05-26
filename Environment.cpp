
#include "Environment.h"
#include "SValue.h"

Environment::Environment( Environment* parent ) : parent( parent )
{}

SValue* Environment::get( const Symbol& sym, SValue* v ) const
{
  auto it = env.find( sym );

  // Found in local environment.
  if ( it != env.end() )
  {
    // Copy value.
    v->value = it->second->value;
  }
  else if ( parent )
  {
    // Try parent.
    parent->get( sym, v );
  }
  else
  {
    error( v, sym.label + " not found" );
  }

  return v;
}

void Environment::set( const Symbol& sym, const SValue& v )
{
  env[ sym ] = std::make_shared< SValue >( v );
}

void Environment::rootSet( const Symbol& s, const SValue& v )
{
  // Find root. An environment with no parent.
  Environment* root = this;
  while ( root->parent )
  {
    root = root->parent;
  }

  root->set( s, v );
}
