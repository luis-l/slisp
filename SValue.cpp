
#include "SValue.h"
#include "Traversal.h"

#include <assert.h>
#include <ostream>

// Does a deep copy of the Lambda.
Lambda copy( const Lambda& l )
{
  return { l.env, std::make_shared< SValue >( *l.formals ), std::make_shared< SValue >( *l.body ) };
}

SValueRef error( const std::string& message, SValueRef v )
{
  v->value = Error( message );
  v->children.clear();
  return v;
}

SValueRef reduce( SValueRef parent, SValueRef child )
{
  parent->value = std::move( child->value );
  parent->children = std::move( child->children );
  return parent;
}

std::unordered_map< const SValue*, std::size_t > getDepths( const SValue& r )
{
  std::unordered_map< const SValue*, std::size_t > depths;
  traverseLevelOrder( r, [ &depths ]( const SValue& v, std::size_t level ) { depths[ &v ] = level; } );
  return depths;
}

std::ostream& showExpression( std::ostream& o, const SValue& r )
{
  int i = 0;
  for ( const auto& child : r.children )
  {
    show( o, *child );
    if ( ++i < r.children.size() )
    {
      o << ' ';
    }
  }

  return o;
}

std::ostream& show( std::ostream& o, const SValue& r )
{
  if ( r.isType< Sexpr >() )
  {
    o << '(';
    showExpression( o, r );
    o << ')';
  }
  else if ( r.isType< QExpr >() )
  {
    o << '{';
    showExpression( o, r );
    o << '}';
  }
  else
  {
    o << r.value;
  }

  return o;
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

std::ostream& operator<<( std::ostream& o, const CoreFunction& f )
{
  return o << "<function>";
}

std::ostream& operator<<( std::ostream& o, const Lambda& f )
{
  o << "\\ ";
  show( o, *f.formals ) << ' ';
  show( o, *f.body );
  return o;
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

SValue& SValue::operation()
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
