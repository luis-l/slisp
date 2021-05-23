
#include "Evaluator.h"
#include "ListOperations.h"
#include "SValue.h"

#include <functional>
#include <numeric>

SValueRef evaluateSexpr( Environment& e, SValueRef s );
SValueRef evaluateNumeric( const std::string& op, SValueRef v );

void addCoreFunctions( Environment& e )
{
  e[ "+" ] =
    std::make_shared< SValue >( []( Environment&, SValueRef v ) -> SValueRef { return evaluateNumeric( "+", v ); } );

  e[ "-" ] =
    std::make_shared< SValue >( []( Environment&, SValueRef v ) -> SValueRef { return evaluateNumeric( "-", v ); } );

  e[ "*" ] =
    std::make_shared< SValue >( []( Environment&, SValueRef v ) -> SValueRef { return evaluateNumeric( "*", v ); } );

  e[ "/" ] =
    std::make_shared< SValue >( []( Environment&, SValueRef v ) -> SValueRef { return evaluateNumeric( "/", v ); } );

  e[ "head" ] = std::make_shared< SValue >( []( Environment&, SValueRef v ) -> SValueRef { return head( v ); } );
  e[ "tail" ] = std::make_shared< SValue >( []( Environment&, SValueRef v ) -> SValueRef { return tail( v ); } );
  e[ "list" ] = std::make_shared< SValue >( []( Environment&, SValueRef v ) -> SValueRef { return list( v ); } );
  e[ "eval" ] = std::make_shared< SValue >( []( Environment&, SValueRef v ) -> SValueRef { return eval( v ); } );
  e[ "join" ] = std::make_shared< SValue >( []( Environment&, SValueRef v ) -> SValueRef { return join( v ); } );
}

SValueRef evaluate( Environment& e, SValueRef s )
{
  // Symbol
  if ( auto symLabel = std::get_if< std::string >( &s->value ) )
  {
    return reduce( s, getSymbol( *symLabel, e, s ) );
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
