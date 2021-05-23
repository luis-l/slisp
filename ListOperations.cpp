
#include "ListOperations.h"

#include "SValue.h"

std::unique_ptr< SValue >& head( std::unique_ptr< SValue >& v )
{
  if ( std::get_if< QExpr >( &v->value ) )
  {
    if ( v->children.empty() )
    {
      v->value = Error( "Nothing passed to head" );
    }
    else
    {
      // Take the front only.
      v->children.erase( v->children.begin() + 1, v->children.end() );
    }
  }

  else
  {
    v->value = Error( "head must take a Q-expression" );
  }

  return v;
}

std::unique_ptr< SValue >& tail( std::unique_ptr< SValue >& v )
{
  if ( std::get_if< QExpr >( &v->value ) )
  {
    if ( v->children.empty() )
    {
      v->value = Error( "Nothing passed to tail" );
    }
    else
    {
      // Remain the front.
      v->children.erase( v->children.begin() );
    }
  }

  else
  {
    v->value = Error( "tail must take a Q-expression" );
  }

  return v;
}

std::unique_ptr< SValue >& list( std::unique_ptr< SValue >& v )
{
  v->value = QExpr();
  v->children.erase( v->children.begin() ); // erase operation of S expression ("list"), only keep arguments
  return v;
}

std::unique_ptr< SValue >& eval( std::unique_ptr< SValue >& v )
{
  if ( std::get_if< QExpr >( &v->value ) )
  {
    v->value = Sexpr();
  }
  else
  {
    v->value = Error( "eval must take a Q-expression" );
  }

  return v;
}

// v is an s-expression containing "join qexpr qexpr ... "
// sexpr
//   join
//   qexpr
//   qexpr
std::unique_ptr< SValue >& join( std::unique_ptr< SValue >& v )
{
  // Must contain operator and at least 1 argument.
  if ( v->children.size() < 2 )
  {
    v->value = Error( "join needs at least 2 arguments" );
    return v;
  }

  const bool allQexprs = std::all_of( v->children.cbegin() + 1, v->children.cend(), []( const auto& child ) {
    return std::get_if< QExpr >( &child->value ) != nullptr;
  } );

  if ( !allQexprs )
  {
    v->value = Error( "join must take Q-expressions" );
  }
  else
  {
    auto& concat = *( v->children.begin() + 1 );

    std::for_each( v->children.begin() + 2, v->children.end(), [ &concat ]( auto& qexprChild ) {
      concat->children.insert(
        concat->children.end(),
        std::make_move_iterator( qexprChild->children.begin() ),
        std::make_move_iterator( qexprChild->children.end() ) );
    } );

    v = std::move( concat );
  }

  return v;
}
