
#include "ListOperations.h"

#include "SValue.h"

#include <algorithm>
#include <numeric>

// v is an S-expression. e.g. (op qexpr)
SValueRef head( SValueRef v )
{
  REQUIRE( v, !v->isEmpty(), "Nothing passed to head" );
  REQUIRE( v, v->argumentCount() == 1, "head only takes 1 argument" );

  SValueRef qexpr = v->arguments().front();

  REQUIRE( v, qexpr->isType< QExpr >(), "head must take a Q-expression" );
  REQUIRE( v, !qexpr->isEmpty(), "head requires a non-empty Q-expression" );

  // Take the front only for the Q-expression children.
  qexpr->children.erase( qexpr->children.begin() + 1, qexpr->children.end() );
  return reduce( v, qexpr );
}

SValueRef tail( SValueRef v )
{
  REQUIRE( v, !v->isEmpty(), "Nothing passed to tail" );
  REQUIRE( v, v->argumentCount() == 1, "tail only takes 1 argument" );

  SValueRef qexpr = v->arguments().front();

  REQUIRE( v, qexpr->isType< QExpr >(), "tail must take a Q-expression" );
  REQUIRE( v, !qexpr->isEmpty(), "tail requires a non-empty Q-expression" );

  // Remove the front.
  qexpr->children.erase( qexpr->children.begin() );
  return reduce( v, qexpr );
  //return qexpr;
}

SValueRef list( SValueRef v )
{
  v->value = QExpr();
  v->children.erase( v->children.begin() ); // Drop the operation "list". Keep args.
  return v;
}

SValueRef eval( SValueRef v )
{
  REQUIRE( v, v->argumentCount() == 1, "eval must take 1 argument" );

  std::span< SValueRef > args = v->arguments();
  SValueRef arg = args.front();

  REQUIRE( v, arg->isType< QExpr >(), "eval must take a Q-expression" );
  v->value = Sexpr();
  v->children = std::move( arg->children ); // Take Q-expression children as S-expression children.
  return v;
}

// v is an s-expression containing "join qexpr qexpr ... "
// sexpr
//   join
//   qexpr
//   qexpr
SValueRef join( SValueRef v )
{
  REQUIRE( v, v->argumentCount() > 0, "join needs at least 1 argument" );

  auto args = v->arguments();

  const bool allQexprs =
    std::all_of( args.begin(), args.end(), []( const auto& child ) { return child->isType< QExpr >(); } );

  REQUIRE( v, allQexprs, "join must take Q-expressions" );

  SValueRef concat = args.front();

  std::for_each( std::next( args.begin() ), args.end(), [ concat ]( auto& child ) {
    concat->children.insert(
      concat->children.end(),
      std::make_move_iterator( child->children.begin() ),
      std::make_move_iterator( child->children.end() ) );

    child->children.clear();
  } );

  return reduce( v, concat );
}
