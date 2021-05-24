#pragma once

#include "SValue.h"
#include "SValueRef.h"

#include <algorithm>
#include <numeric>
#include <string>
#include <type_traits>

// v is an S-expression. e.g. + 1 2 3 5
template < typename NumericT >
SValueRef evaluateNumericT( const std::string& op, SValueRef v )
{
  static_assert(
    std::is_integral_v< NumericT > || std::is_floating_point_v< NumericT >,
    "evaluateNumeric must be used with integral or floating point types" );

  auto args = v->arguments();

  const bool allNumeric =
    std::all_of( args.begin(), args.end(), []( const SValueRef& s ) { return s->isType< NumericT >(); } );

  REQUIRE( v, allNumeric, "Not all arguments are the same numeric type" );

  // Negation
  if ( op == "-" && v->argumentCount() == 1 )
  {
    auto& arg = args.front();
    arg->apply< NumericT >( std::negate< NumericT >() );
    return reduce( v, arg );
  }

  using AccumulatorFunc = std::function< SValueRef( SValueRef, SValueRef ) >;

  auto integralOperator = [ &op ]() -> AccumulatorFunc {
    if ( op == "+" )
    {
      return
        []( SValueRef x, SValueRef y ) -> SValueRef { return concat< NumericT >( x, y, std::plus< NumericT >() ); };
    }
    if ( op == "-" )
    {
      return
        []( SValueRef x, SValueRef y ) -> SValueRef { return concat< NumericT >( x, y, std::minus< NumericT >() ); };
    }
    if ( op == "*" )
    {
      return []( SValueRef x, SValueRef y ) -> SValueRef {
        return concat< NumericT >( x, y, std::multiplies< NumericT >() );
      };
    }
    if ( op == "/" )
    {
      return []( SValueRef x, SValueRef y ) -> SValueRef {
        if ( x->isType< Error >() )
        {
          return x; // Propagate error.
        }

        NumericT yvalue = std::get< NumericT >( y->value );
        REQUIRE( x, yvalue != NumericT{ 0 }, "Division by zero" );
        return concat< NumericT >( x, y, std::divides< NumericT >() );
      };
    }

    return []( SValueRef x, SValueRef ) -> SValueRef { return error( "Unsupported numerical operator", x ); };
  };

  return reduce( v, std::accumulate( args.begin() + 1, args.end(), args.front(), integralOperator() ) );
}
