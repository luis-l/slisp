#pragma once

#include "SValue.h"

#include <algorithm>
//#include <numeric>
#include <string>
#include <type_traits>

//template < typename Itr >
//class DataPointerIterator
//{
//  Itr itr;
//
//public:
//  constexpr explicit DataPointerIterator( Itr itr ) : itr( itr )
//  {}
//
//  constexpr auto operator*()
//  {
//    return ( *itr ).get();
//  }
//
//  constexpr auto operator++()
//  {
//    ++itr;
//    return *this;
//  }
//  constexpr friend bool operator!=( DataPointerIterator< Itr > a, DataPointerIterator< Itr > b )
//  {
//    return a.itr != b.itr;
//  }
//};

// v is an S-expression. e.g. 1 2 3 5
template < typename NumericT >
SValue* evaluateNumericT( const std::string& op, SValue* v )
{
  static_assert(
    std::is_integral_v< NumericT > || std::is_floating_point_v< NumericT >,
    "evaluateNumeric must be used with integral or floating point types" );

  Cells& cells = v->cellsRequired();

  const bool allNumeric =
    std::all_of( cells.begin(), cells.end(), []( const auto& s ) { return s->isType< NumericT >(); } );

  REQUIRE( v, allNumeric, op + " Not all arguments are the same numeric type" );

  // Negation
  if ( op == "-" && cells.size() == 1 )
  {
    SValue* s = cells.front();
    v->value = apply< NumericT >( s, std::negate< NumericT >() );
    return v;
  }

  using AccumulatorFunc = std::function< Value( NumericT, NumericT ) >;

  auto pickAccumulator = [ &op ]() -> AccumulatorFunc {
    if ( op == "+" )
    {
      return []( NumericT x, NumericT y ) -> Value { return x + y; };
    }
    if ( op == "-" )
    {
      return []( NumericT x, NumericT y ) -> Value { return x - y; };
    }
    if ( op == "*" )
    {
      return []( NumericT x, NumericT y ) -> Value { return x * y; };
    }
    if ( op == "/" )
    {
      return []( NumericT x, NumericT y ) -> Value {
        return y == 0 ? Value( Error( "Division by zero" ) ) : Value( x / y );
      };
    }
    return []( NumericT x, NumericT y ) -> Value { return Error( "Unsupported numerical operator" ); };
  };

  //auto integralOperator = [ &op ]() -> AccumulatorFunc {
  //  if ( op == "+" )
  //  {
  //    return []( SValue* x, SValue* y ) -> SValue* { return concat< NumericT >( x, y, std::plus< NumericT >() ); };
  //  }
  //  if ( op == "-" )
  //  {
  //    return []( SValue* x, SValue* y ) -> SValue* { return concat< NumericT >( x, y, std::minus< NumericT >() ); };
  //  }
  //  if ( op == "*" )
  //  {
  //    return
  //      []( SValue* x, SValue* y ) -> SValue* { return concat< NumericT >( x, y, std::multiplies< NumericT >() ); };
  //  }
  //  if ( op == "/" )
  //  {
  //    return []( SValue* x, SValue* y ) -> SValue* {
  //      if ( x->isError() )
  //      {
  //        return x; // Propagate error.
  //      }

  //      NumericT yvalue = std::get< NumericT >( y->value );
  //      REQUIRE( x, yvalue != NumericT{ 0 }, "Division by zero" );
  //      return concat< NumericT >( x, y, std::divides< NumericT >() );
  //    };
  //  }

  //  return []( SValue* x, SValue* ) -> SValue* { return error( x, "Unsupported numerical operator" ); };
  //};

  //SValue* finalValue = std::accumulate(
  //  DataPointerIterator( cells.begin() + 1 ), DataPointerIterator( cells.end() ), cells.front(), integralOperator() );

  std::unique_ptr< SValue > first = cells.takeFront();
  Value result = first->value;
  auto accumulator = pickAccumulator();

  while ( !cells.isEmpty() )
  {
    result = accumulator( std::get< NumericT >( result ), cells.takeFront()->get< NumericT >() );
    if ( auto error = std::get_if< Error >( &result ) )
    {
      break;
    }
  }

  v->value = result;
  return v;
}
