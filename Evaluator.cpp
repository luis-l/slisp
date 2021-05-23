
#include "Evaluator.h"
#include "ListOperations.h"
#include "SValue.h"

#include <functional>
#include <numeric>

std::unique_ptr< SValue >& evaluateSexpr( std::unique_ptr< SValue >& s );
std::unique_ptr< SValue >& evaluateOperation( const std::string& op, std::unique_ptr< SValue >& v );
std::unique_ptr< SValue >& evaluateNumeric( const std::string& op, SValueIt begin, SValueIt end );

std::unique_ptr< SValue >& evaluate( std::unique_ptr< SValue >& s )
{
  if ( std::get_if< Sexpr >( &s->value ) )
  {
    return evaluateSexpr( s );
  }

  return s;
}

std::unique_ptr< SValue >& evaluateSexpr( std::unique_ptr< SValue >& s )
{
  for ( auto& child : s->children )
  {
    child = std::move( evaluate( child ) );
  }

  // Atom.
  if ( s->children.empty() )
  {
    return s;
  }

  // nested superfluous expressions. e.g. ( (+ 1 2) )
  if ( s->children.size() == 1 )
  {
    s = std::move( s->children.front() );
    return s;
  }

  if ( auto op = std::get_if< std::string >( &s->children.front()->value ) )
  {
    return evaluateOperation( *op, s );
  }
  else
  {
    s->value = Error( "Unsupported operator" );
    return s;
  }
}

std::unique_ptr< SValue >& evaluateOperation( const std::string& op, std::unique_ptr< SValue >& v )
{
  auto firstArgIt = v->children.begin() + 1;

  if ( op == "head" )
  {
    // Children must only contain op and qexpr
    v = std::move( head( *firstArgIt ) );
  }
  else if ( op == "tail" )
  {
    // Children must only contain op and qexpr
    v = std::move( tail( *firstArgIt ) );
  }
  else if ( op == "list" )
  {
    v = std::move( list( v ) );
  }
  else if ( op == "eval" )
  {
    // Children must only contain op and qexpr. e.g. eval {1 2}
    v = std::move( evaluate( eval( *firstArgIt ) ) );
  }
  else if ( op == "join" )
  {
    v = std::move( join( v ) );
  }
  else
  {
    // Children must only contain op and one or more numeric arguments
    v = std::move( evaluateNumeric( op, firstArgIt, v->children.end() ) );
  }

  return v;
}

std::unique_ptr< SValue >& evaluateNumeric( const std::string& op, SValueIt begin, SValueIt end )
{
  const bool allIntegral = std::all_of(
    begin, end, []( const std::unique_ptr< SValue >& s ) { return std::get_if< int >( &s->value ) != nullptr; } );

  if ( !allIntegral )
  {
    ( *begin )->value = Error( "Not all arguments are integral" );
    return ( *begin );
  }

  // Negation
  if ( op == "-" && ( end - begin == 1 ) )
  {
    ( *begin )->value = -std::get< int >( ( *begin )->value );
    return ( *begin );
  }

  auto integralOperator = [ &op ]() -> std::function< Value( const Value&, const std::unique_ptr< SValue >& ) > {
    if ( op == "+" )
    {
      return []( const Value& x, const std::unique_ptr< SValue >& y ) {
        return Value( std::get< int >( x ) + std::get< int >( y->value ) );
      };
    }
    if ( op == "-" )
    {
      return []( const Value& x, const std::unique_ptr< SValue >& y ) {
        return Value( std::get< int >( x ) - std::get< int >( y->value ) );
      };
    }
    if ( op == "*" )
    {
      return []( const Value& x, const std::unique_ptr< SValue >& y ) {
        return Value( std::get< int >( x ) * std::get< int >( y->value ) );
      };
    }
    if ( op == "/" )
    {
      return []( const Value& x, const std::unique_ptr< SValue >& y ) {
        if ( std::get_if< Error >( &x ) )
        {
          return x; // Propagate error.
        }
        int yint = std::get< int >( y->value );
        return yint == 0 ? Value( Error( "Division by zero" ) ) : Value( std::get< int >( x ) / yint );
      };
    }

    return []( const Value& x, const std::unique_ptr< SValue >& y ) { return Value( Error( "Unknown operator" ) ); };
  };

  ( *begin )->value = std::accumulate( begin + 1, end, ( *begin )->value, integralOperator() );
  return *begin;
}
