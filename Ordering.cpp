
#include "Ordering.h"
#include "SValue.h"

template < typename T, typename CompareOp >
SValue* evaluateCompare( SValue* v, CompareOp compare )
{
  REQUIRE( v, v->size() == 2, "Expects two arguments" );

  Cells& cells = v->cellsRequired();
  std::unique_ptr< SValue > left = cells.takeFront();
  REQUIRE( v, left->isType< T >(), "Got incorrect type" );

  std::unique_ptr< SValue > right = cells.takeFront();
  REQUIRE( v, right->isType< T >(), "Got incorrect type" );

  v->value = compare( left->get< T >(), right->get< T >() ) ? Boolean::True : Boolean::False;
  return v;
}

template < typename CompareOp >
SValue* evalCellsCompare( SValue* v, CompareOp binaryOp )
{
  Cells& cells = v->cellsRequired();
  if ( cells.front()->isType< int >() )
  {
    return evaluateCompare< int >( v, binaryOp );
  }
  return evaluateCompare< double >( v, binaryOp );
}

SValue* evalLesser( Environment& e, SValue* v )
{
  return evalCellsCompare( v, std::less<>() );
}

SValue* evalLesserEqual( Environment& e, SValue* v )
{
  return evalCellsCompare( v, std::less_equal<>() );
}

SValue* evalGreater( Environment& e, SValue* v )
{
  return evalCellsCompare( v, std::greater<>() );
}

SValue* evalGreaterEqual( Environment& e, SValue* v )
{
  return evalCellsCompare( v, std::greater_equal<>() );
}

SValue* evalEquality( Environment& e, SValue* v )
{
  Cells& cells = v->cellsRequired();
  std::unique_ptr< SValue > first = cells.takeFront();
  while ( !cells.isEmpty() )
  {
    std::unique_ptr< SValue > other = cells.takeFront();
    const bool isEqual = *first == *other;
    if ( !isEqual )
    {
      v->value = Boolean::False;
      return v;
    }
  }

  // All equal.
  v->value = Boolean::True;
  return v;
}

SValue* evalNotEqual( Environment& e, SValue* v )
{
  v = evalEquality( e, v );
  v->value = v->get< Boolean >() == Boolean::True ? Boolean::False : Boolean::True;
  return v;
}
