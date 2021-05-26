#pragma once

#include "Environment.h"
#include "Symbol.h"

#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

class SValue;

struct Error
{
  std::string message;
};

// \  { x y }         {+ x y}
//    formals qexpr   body qexpr
struct Lambda
{
  Lambda() = default;

  Lambda( Environment e, std::unique_ptr< SValue > formals, std::unique_ptr< SValue > body )
  : env( std::move( e ) ), formals( std::move( formals ) ), body( std::move( body ) )
  {}

  Lambda( const Lambda& other )
  {
    *this = other;
  }

  Lambda& operator=( const Lambda& other )
  {
    if ( this != &other )
    {
      env = other.env;
      formals = std::make_unique< SValue >( *other.formals );
      body = std::make_unique< SValue >( *other.body );
    }
    return *this;
  }

  Lambda( Lambda&& other ) noexcept
  {
    *this = std::move( other );
  }

  Lambda& operator=( Lambda&& other ) noexcept
  {
    if ( this != &other )
    {
      env = std::move( other.env );
      formals = std::move( other.formals );
      body = std::move( other.body );
    }
    return *this;
  }

  Environment env;
  std::unique_ptr< SValue > formals;
  std::unique_ptr< SValue > body;
};

// Cells are Semi-Regular type.
class Cells
{
public:
  using ValueT = std::vector< std::unique_ptr< SValue > >;

  Cells() = default;
  Cells( ValueT data ) : data( std::move( data ) )
  {}

  Cells( const Cells& other );
  Cells& operator=( const Cells& other );

  Cells( Cells&& other ) noexcept;
  Cells& operator=( Cells&& other ) noexcept;

  std::size_t size() const;

  bool isEmpty() const;

  const ValueT& children() const;
  ValueT& children();

  void append( std::unique_ptr< SValue > v );

  std::unique_ptr< SValue > takeFront();

  void drop( ValueT::iterator begin, ValueT::iterator end );
  void drop( ValueT::iterator pos )
  {
    data.erase( pos );
  }

  void clear()
  {
    data.clear();
  }

  SValue* operator[]( std::size_t index )
  {
    return data[ index ].get();
  }

  const SValue* operator[]( std::size_t index ) const
  {
    return data[ index ].get();
  }

  SValue* front()
  {
    return data.front().get();
  }

  const SValue* front() const
  {
    return data.front().get();
  }

  SValue* back();
  const SValue* back() const;

  ValueT::iterator begin()
  {
    return data.begin();
  }

  ValueT::iterator end()
  {
    return data.end();
  }

private:
  ValueT data;
};

/// A built-in function.
/// The environment can be modified by the function.
/// The S-expression can be modified and reduced (evaluated) by the function.
/// It returns the new, evalauted S-expression.
using CoreFunction = std::function< SValue*( Environment&, SValue* v ) >;

struct QExpr
{
  // Q-expressions contain cells that are not evaluated.
  Cells cells;
};

using Value = std::variant< Cells, QExpr, CoreFunction, Symbol, Lambda, int, double, Error >;

class SValue
{
public:
  Value value;

  bool isExpressionType() const;
  bool isSExpression() const;
  bool isQExpression() const;

  bool isError() const;

  /// Get the cell children for an S-expression or Q-expression. Null for other types.
  const Cells* cells() const;
  Cells* cells();

  // Gets the cells for the given S-expression or Q-expression.
  // For other types, an assertion fails.
  Cells& cellsRequired();

  /// Checks if the S-expression or Q-expression is empty. For other types, it is always true.
  bool isEmpty() const;

  /// Get the children count for S-expressions or Q-expressions. For other types, it is always 0.
  std::size_t size() const;

  template < typename T >
  bool isType() const
  {
    return std::get_if< T >( &value ) != nullptr;
  }

  template < typename T >
  auto getIf()
  {
    return std::get_if< T >( &value );
  }

  template < typename T >
  auto getIf() const
  {
    return std::get_if< T >( &value );
  }

  template < typename T >
  auto& get()
  {
    return std::get< T >( value );
  }

  template < typename T >
  auto& get() const
  {
    return std::get< T >( value );
  }

  /// Apply a function on the expression value.
  template < typename T, typename FuncT >
  void apply( FuncT f )
  {
    value = f( std::get< T >( value ) );
  }

  /// Apply a function for each child. const SValue& is passed to the function.
  template < typename ApplyF >
  void foreachCell( ApplyF f ) const
  {
    if ( const Cells* c = cells() )
    {
      for ( const auto& child : c->children() )
      {
        f( *child );
      }
    }
  }

  /// Apply a function for each child. SValue& is passed to the function.
  template < typename ApplyF >
  void foreachCell( ApplyF f )
  {
    if ( Cells* c = cells() )
    {
      for ( auto& child : c->children() )
      {
        f( *child );
      }
    }
  }
};

template < typename T, typename ConcatF >
SValue* concat( SValue* left, SValue* right, ConcatF concatFunc )
{
  left->value = concatFunc( std::get< T >( left->value ), std::get< T >( right->value ) );
  return left;
}

template < typename T >
std::unique_ptr< SValue > makeSValue( T&& t )
{
  return std::make_unique< SValue >( std::forward< T >( t ) );
}

std::unique_ptr< SValue > makeDefaultSValue();

SValue* error( SValue* s, const std::string& message );
SValue* empty( SValue* s );

// Apply a function on the value for s. The function result is returned and s is not modified.
template < typename T, typename ApplyF >
Value apply( SValue* s, ApplyF f )
{
  return f( std::get< T >( s->value ) );
}

std::unordered_map< const SValue*, std::size_t > getDepths( const SValue& r );

/// @brief Show the SValue as an expression string.
std::ostream& show( std::ostream& o, const SValue& r );

/// @brief Show the SValue as a tree.
std::ostream& operator<<( std::ostream& o, const SValue& r );
std::ostream& operator<<( std::ostream& o, const Cells& t );
std::ostream& operator<<( std::ostream& o, const QExpr& t );
std::ostream& operator<<( std::ostream& o, const Error& e );
std::ostream& operator<<( std::ostream& o, const CoreFunction& f );
std::ostream& operator<<( std::ostream& o, const Lambda& f );

template < typename... Args >
std::ostream& operator<<( std::ostream& o, const std::variant< Args... >& value )
{
  std::visit( [ &o ]( const auto& item ) { o << item; }, value );
  return o;
}

/// Requires that the condition is satisfied.
/// If not then an Error is returned.
/// Usage: REQUIRE( !value->isEmpty(), "Must not be empty" );
#define REQUIRE( svalue, condition, message ) \
  if ( !( condition ) ) \
  { \
    return error( ( svalue ), ( message ) ); \
  }
