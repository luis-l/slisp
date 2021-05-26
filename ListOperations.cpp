
#include "ListOperations.h"
#include "Evaluator.h"

#include "SValue.h"

#include <algorithm>

// v is an expression containing { 1 2 3 }
SValue* head( SValue* v )
{
  Cells& args = v->cellsRequired();
  REQUIRE( v, args.size() == 1, "head requires 1 argument" );

  SValue* qexpr = args.front();
  REQUIRE( v, qexpr->isQExpression(), "head expects a QExpression" );
  REQUIRE( v, !qexpr->isEmpty(), "head expects a non-empty QExpression" );

  Cells& qexprCells = qexpr->cellsRequired();

  // Take the front only for the Q-expression children.
  qexprCells.drop( qexprCells.begin() + 1, qexprCells.end() );

  // V becomes Q-expression.
  std::swap( *v, *qexpr );
  return v;
}

// v is an expression containing { 1 2 3 }
SValue* tail( SValue* v )
{
  Cells& args = v->cellsRequired();
  REQUIRE( v, args.size() == 1, "tail requires 1 argument" );

  SValue* qexpr = args.front();
  REQUIRE( v, qexpr->isQExpression(), "tail expects a QExpression" );

  Cells& qexprCells = qexpr->cellsRequired();

  if ( qexprCells.isEmpty() )
  {
    v->value = QExpr();
    return v;
  }

  // Remove the front.
  qexprCells.drop( qexprCells.begin() );

  // V becomes Q-expression.
  //std::swap( *v, *qexpr );
  v->value = QExpr( std::move( qexprCells ) );
  return v;
}

SValue* list( SValue* v )
{
  REQUIRE( v, v->isSExpression(), "list expects an S-expression" );
  v->value = QExpr{ std::move( v->cellsRequired() ) }; // Move the S-expression cells into a Q-expression.
  return v;
}

// v is an s-expression containing "qexpr qexpr ... "
// sexpr
//   qexpr
//   qexpr
SValue* join( SValue* v )
{
  Cells& cells = v->cellsRequired();

  const bool allQexprs =
    std::all_of( cells.begin(), cells.end(), []( const auto& child ) { return child->isQExpression(); } );

  REQUIRE( v, allQexprs, "join must take Q-expressions" );

  std::unique_ptr< SValue > joined = cells.takeFront();
  Cells& joinedCells = joined->cellsRequired();

  // Join rest of the cells.
  std::for_each( cells.begin(), cells.end(), [ &joinedCells ]( auto& child ) {
    Cells& otherCells = child->cellsRequired();
    while ( !otherCells.isEmpty() )
    {
      joinedCells.append( otherCells.takeFront() );
    }
  } );

  // v now becomes a Q-expression.
  std::swap( *v, *joined );
  return v;
}
