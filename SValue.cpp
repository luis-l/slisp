
#include "SValue.h"

#include <assert.h>
#include <ostream>

SValueRef error( const std::string& message, SValueRef v )
{
  v->value = Error( message );
  v->children.clear();
  return v;
}

SValueRef reduce( SValueRef& parent, SValueRef child )
{
  parent = std::move( child );
  return parent;
}

SValueRef getSymbol( const std::string& sym, Environment& e, SValueRef v )
{
  auto it = e.find( sym );
  REQUIRE( v, it != e.end(), sym + " not found" );
  return it->second;
}

std::unordered_map< const SValue*, std::size_t > getDepths( const SValue& r )
{
  std::unordered_map< const SValue*, std::size_t > depths;
  traverseLevelOrder( r, [ &depths ]( const SValue& v, std::size_t level ) { depths[ &v ] = level; } );
  return depths;
}

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
  return o << "Error: " << e.message;
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

std::ostream& operator<<( std::ostream& o, const CoreFunction& f )
{
  return o << "<function>";
}

std::size_t SValue::size() const
{
  return children.size();
}

bool SValue::isEmpty() const
{
  return children.empty();
}

std::size_t SValue::argumentCount() const
{
  // Subtract one since the first arugment is the operation.
  return children.empty() ? 0 : children.size() - 1;
}

/// @brief Get the operation for S-expression.

const SValue& SValue::operation() const
{
  return *children.front();
}

std::span< SValueRef > SValue::arguments()
{
  return isEmpty() ? std::span{ children.begin(), 0 } : std::span{ children.begin() + 1, children.end() };
}

bool SValue::isError() const
{
  return isType< Error >();
}
