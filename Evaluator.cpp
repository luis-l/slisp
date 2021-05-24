
#include "Evaluator.h"
#include "ListOperations.h"
#include "SValue.h"

#include <functional>
#include <numeric>

SValueRef evaluateSexpr( Environment& e, SValueRef s );
SValueRef evaluateNumeric( const std::string& op, SValueRef v );
SValueRef evaluateDef( Environment& e, SValueRef v );

void addCoreFunctions( Environment& e )
{
  auto bindNumericOp = []( const std::string& op ) {
    return [ op ]( Environment&, SValueRef v ) -> SValueRef { return evaluateNumeric( op, v ); };
  };

  e[ "+" ] = std::make_shared< SValue >( bindNumericOp( "+" ) );
  e[ "-" ] = std::make_shared< SValue >( bindNumericOp( "-" ) );
  e[ "*" ] = std::make_shared< SValue >( bindNumericOp( "*" ) );
  e[ "/" ] = std::make_shared< SValue >( bindNumericOp( "/" ) );

  auto bindListOp = []( auto f ) { return [ f ]( Environment&, SValueRef v ) -> SValueRef { return f( v ); }; };

  e[ "head" ] = std::make_shared< SValue >( bindListOp( head ) );
  e[ "tail" ] = std::make_shared< SValue >( bindListOp( tail ) );
  e[ "list" ] = std::make_shared< SValue >( bindListOp( list ) );
  e[ "join" ] = std::make_shared< SValue >( bindListOp( join ) );

  e[ "eval" ] =
    std::make_shared< SValue >( []( Environment& e, SValueRef v ) -> SValueRef { return evaluate( e, eval( v ) ); } );

  e[ "def" ] = std::make_shared< SValue >( evaluateDef );
}

SValueRef evaluate( Environment& e, SValueRef s )
{
  // Symbol
  if ( auto symLabel = std::get_if< std::string >( &s->value ) )
  {
    return reduce( s, getFromEnv( *symLabel, e, s ) );
  }

  if ( s->isType< Sexpr >() )
  {
    return evaluateSexpr( e, s );
  }

  return s;
}

SValueRef evaluateSexpr( Environment& e, SValueRef s )
{
  for ( auto& child : s->children )
  {
    reduce( child, evaluate( e, child ) );
  }

  // Atom.
  if ( s->isEmpty() )
  {
    return s;
  }

  // nested superfluous expressions. e.g. ( ( ) )
  if ( s->size() == 1 )
  {
    return reduce( s, s->children.front() );
  }

  if ( auto callable = std::get_if< CoreFunction >( &s->operation().value ) )
  {
    return ( *callable )( e, s );
  }
  else
  {
    return error( "Operation is not callable", s );
  }
}

SValueRef evaluateDef( Environment& e, SValueRef v )
{
  REQUIRE( v, !v->isEmpty(), "Nothing passed to def" );
  REQUIRE( v, v->argumentCount() >= 2, "def requires 2 or more arguments" );

  // def { x y }   10 20
  //      symbols  expressions
  // x = 10, y = 20
  std::span< SValueRef > args = v->arguments();
  SValueRef symbols = args.front();
  std::span< SValueRef > expressions{ std::next( args.begin() ), args.end() };

  REQUIRE( v, symbols->isType< QExpr >(), "def must take a Q-expression for the first argument" );
  REQUIRE( v, !symbols->isEmpty(), "def requires a non-empty Q-expression for first argument" );
  REQUIRE( v, symbols->size() == expressions.size(), "Symbol count must match expression count" );

  const bool allSymbolsAreSymbolType = std::all_of(
    symbols->children.cbegin(), symbols->children.cend(), []( const auto& c ) { return c->isType< std::string >(); } );

  REQUIRE( v, allSymbolsAreSymbolType, "Cannot define for non-symbol type" );

  for ( std::size_t i = 0; i < symbols->size(); ++i )
  {
    addToEnv( e, std::get< std::string >( symbols->children[ i ]->value ), expressions[ i ] );
  }

  v->value = Sexpr();
  v->children.clear();
  return v;
}

// v is an S-expression. e.g. + 1 2 3 5
SValueRef evaluateNumeric( const std::string& op, SValueRef v )
{
  auto args = v->arguments();

  const bool allIntegral =
    std::all_of( args.begin(), args.end(), []( const SValueRef s ) { return s->isType< int >(); } );

  REQUIRE( v, allIntegral, "Not all arguments are integral" );

  // Negation
  if ( op == "-" && v->argumentCount() == 1 )
  {
    auto& arg = args.front();
    arg->apply< int >( std::negate< int >() );
    return reduce( v, arg );
  }

  using AccumulatorFunc = std::function< SValueRef( SValueRef, SValueRef ) >;

  auto integralOperator = [ &op ]() -> AccumulatorFunc {
    if ( op == "+" )
    {
      return []( SValueRef x, SValueRef y ) -> SValueRef { return concat< int >( x, y, std::plus< int >() ); };
    }
    if ( op == "-" )
    {
      return []( SValueRef x, SValueRef y ) -> SValueRef { return concat< int >( x, y, std::minus< int >() ); };
    }
    if ( op == "*" )
    {
      return []( SValueRef x, SValueRef y ) -> SValueRef { return concat< int >( x, y, std::multiplies< int >() ); };
    }
    if ( op == "/" )
    {
      return []( SValueRef x, SValueRef y ) -> SValueRef {
        if ( x->isType< Error >() )
        {
          return x; // Propagate error.
        }

        int yvalue = std::get< int >( y->value );
        REQUIRE( x, yvalue != 0, "Division by zero" );
        return concat< int >( x, y, std::divides< int >() );
      };
    }

    return []( SValueRef x, SValueRef ) -> SValueRef { return error( "Unsupported numerical operator", x ); };
  };

  return reduce( v, std::accumulate( args.begin() + 1, args.end(), args.front(), integralOperator() ) );
}
