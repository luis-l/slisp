
#include "Evaluator.h"
#include "SValue.h"

#include <functional>
#include <numeric>

std::unique_ptr< SValue >& evaluate( std::unique_ptr< SValue >& s )
{
  if ( std::get_if< ExpressionType >( &s->value ) )
  {
    return evaluateSexpr( s );
  }

  return s;
}

std::unique_ptr< SValue >& evaluateSexpr( std::unique_ptr< SValue >& s )
{
  for ( auto& child : s->children )
  {
    child->value = evaluate( child )->value;
    child->children.clear(); // Done with this - memory can be freed.
  }

  // Atom.
  if ( s->children.empty() )
  {
    return s;
  }

  // nested superfluous expressions. e.g. ( (+ 1 2) )
  if ( s->children.size() == 1 )
  {
    return s->children.front();
  }

  if ( auto op = std::get_if< std::string >( &s->children.front()->value ) )
  {
    s->value = evalOp( *op, s->children.begin() + 1, s->children.end() )->value;
    s->children.clear();
    return s;
  }
  else
  {
    s->value = Error( "Unsupported operator" );
    return s;
  }
}

std::unique_ptr< SValue >& evalOp( const std::string& op, SValueIt begin, SValueIt end )
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
