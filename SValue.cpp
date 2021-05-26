
#include "SValue.h"
#include "Traversal.h"

#include <assert.h>
#include <ostream>

Cells::Cells( const Cells& other )
{
  *this = other;
}

Cells& Cells::operator=( const Cells& other )
{
  if ( this != &other )
  {
    data.reserve( other.size() );
    for ( const auto& i : other.data )
    {
      data.push_back( std::make_unique< SValue >( *i ) );
    }
  }
  return *this;
}

Cells::Cells( Cells&& other ) noexcept
{
  *this = std::move( other );
}

Cells& Cells::operator=( Cells&& other ) noexcept
{
  if ( this != &other )
  {
    data = std::move( other.data );
  }
  return *this;
}

std::size_t Cells::size() const
{
  return data.size();
}

bool Cells::isEmpty() const
{
  return data.empty();
}

const Cells::ValueT& Cells::children() const
{
  return data;
}

Cells::ValueT& Cells::children()
{
  return data;
}

void Cells::append( std::unique_ptr< SValue > v )
{
  data.push_back( std::move( v ) );
}

std::unique_ptr< SValue > Cells::takeFront()
{
  std::unique_ptr< SValue > front = std::move( data.front() );
  data.erase( data.begin() );
  return front;
}

void Cells::drop( ValueT::iterator begin, ValueT::iterator end )
{
  data.erase( begin, end );
}

SValue* Cells::back()
{
  return data.back().get();
}

const SValue* Cells::back() const
{
  return data.back().get();
}

std::unique_ptr< SValue > makeDefaultSValue()
{
  return std::make_unique< SValue >();
}

SValue* error( SValue* s, const std::string& message )
{
  s->value = Error( message );
  return s;
}

SValue* empty( SValue* s )
{
  s->value = Cells();
  return s;
}

std::unordered_map< const SValue*, std::size_t > getDepths( const SValue& r )
{
  std::unordered_map< const SValue*, std::size_t > depths;
  traverseLevelOrder( r, [ &depths ]( const SValue& v, std::size_t level ) { depths[ &v ] = level; } );
  return depths;
}

std::ostream& showExpression( std::ostream& o, const SValue& r )
{
  r.foreachCell( [ &o, count = r.size(), i = 0 ]( const SValue& child ) mutable {
    show( o, child );
    if ( ++i < count ) o << ' ';
  } );

  return o;
}

std::ostream& show( std::ostream& o, const SValue& r )
{
  if ( r.isSExpression() )
  {
    o << '(';
    showExpression( o, r );
    o << ')';
  }
  else if ( r.isQExpression() )
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

std::ostream& operator<<( std::ostream& o, const Cells& t )
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

bool SValue::isExpressionType() const
{
  return isSExpression() || isQExpression();
}

bool SValue::isSExpression() const
{
  return isType< Cells >();
}

bool SValue::isQExpression() const
{
  return isType< QExpr >();
}

bool SValue::isError() const
{
  return isType< Error >();
}

/// Get the cell children for an S-expression or Q-expression. Null for other types.

const Cells* SValue::cells() const
{
  if ( auto sexpr = getIf< Cells >() )
  {
    return sexpr;
  }

  else if ( auto qexpr = getIf< QExpr >() )
  {
    return &qexpr->cells;
  }

  return nullptr;
}

Cells* SValue::cells()
{
  if ( auto sexpr = getIf< Cells >() )
  {
    return sexpr;
  }

  else if ( auto qexpr = getIf< QExpr >() )
  {
    return &qexpr->cells;
  }

  return nullptr;
}

// Gets the cells for the given S-expression or Q-expression.
// For other types, an assertion fails.

Cells& SValue::cellsRequired()
{
  if ( auto sexpr = getIf< Cells >() )
  {
    return *sexpr;
  }

  // Fall back to Q-expr.
  auto qexpr = getIf< QExpr >();
  assert( qexpr != nullptr );
  return qexpr->cells;
}

bool SValue::isEmpty() const
{
  return size() == 0 ? true : false;
}

std::size_t SValue::size() const
{
  const Cells* c = cells();
  return c ? c->size() : 0;
}
