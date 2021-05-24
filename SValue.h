#pragma once

#include "Environment.h"
#include "SValueRef.h"

#include <functional>
#include <memory>
#include <span>
#include <string>
#include <variant>
#include <vector>

struct Sexpr
{};

struct QExpr
{};

struct Error
{
  std::string message;
};

using CoreFunction = std::function< SValueRef( Environment&, SValueRef ) >;

using Value = std::variant< Sexpr, QExpr, CoreFunction, std::string, int, Error >;

using Cells = std::vector< SValueRef >;

class SValue
{
public:
  SValue() = default;
  SValue( Value v ) : value( std::move( v ) )
  {}
  Value value;
  Cells children;

  /// @brief The number of children.
  /// For example, the S-expression: (+ 1 3) , has a size of 3.
  std::size_t size() const;

  bool isEmpty() const;
  std::size_t argumentCount() const;

  /// @brief Get the operation for S-expression.
  const SValue& operation() const;

  std::span< SValueRef > arguments();

  bool isError() const;

  template < typename T >
  bool isType() const
  {
    return std::get_if< T >( &value ) != nullptr;
  }

  /// Apply a function on the expression value.
  template < typename T, typename FuncT >
  void apply( FuncT f )
  {
    value = f( std::get< T >( value ) );
  }
};

template < typename T, typename ConcatF >
SValueRef concat( SValueRef left, SValueRef right, ConcatF concatFunc )
{
  left->value = concatFunc( std::get< T >( left->value ), std::get< T >( right->value ) );
  return left;
}

/// @brief Create an error for the given S-expression.
SValueRef error( const std::string& message, SValueRef v );

/// @brief The parent "becomes" the child. This is used to evaluate and reduce S-expressions.
/// As shown, the inner S-expressions are evaluated and reduced to smaller S-expressions.
/// @code ( + (* 10 10 ) ( - 5 2 ) ) -> ( + 100 3 ) -> 103
SValueRef reduce( SValueRef parent, SValueRef child );

std::unordered_map< const SValue*, std::size_t > getDepths( const SValue& r );

/// @brief Show the SValue as an expression string.
std::ostream& show( std::ostream& o, const SValue& r );

/// @brief Show the SValue as a tree.
std::ostream& operator<<( std::ostream& o, const SValue& r );

std::ostream& operator<<( std::ostream& o, const Sexpr& t );
std::ostream& operator<<( std::ostream& o, const QExpr& t );
std::ostream& operator<<( std::ostream& o, const Error& e );
std::ostream& operator<<( std::ostream& o, const CoreFunction& f );

template < typename... Args >
std::ostream& operator<<( std::ostream& o, const std::variant< Args... >& value )
{
  std::visit( [ &o ]( const auto& item ) { o << item; }, value );
  return o;
}

/// Requires that the condition is satisfied.
/// If not then an Error is returned.
/// Usage: REQUIRE( value, !value->isEmpty(), "Must not be empty" );
#define REQUIRE( svalue, condition, message ) \
  if ( !( condition ) ) \
  { \
    return error( ( message ), ( svalue ) ); \
  }
