#pragma once

#include "SValue.h"

#include <queue>
#include <stack>

template < typename UnaryOp >
void traversePreorder( const SValue& r, UnaryOp f )
{
  std::stack< const SValue* > traversal;
  traversal.push( &r );
  while ( !traversal.empty() )
  {
    const SValue* n = traversal.top();
    traversal.pop();
    f( *n );
    for ( const auto& child : n->children )
    {
      traversal.push( child.get() );
    }
  }
}

template < typename BinaryOp >
void traverseLevelOrder( const SValue& r, BinaryOp f )
{
  std::queue< const SValue* > traversal;
  traversal.push( &r );
  std::size_t level = 0;
  std::size_t queuedCount = 0;
  while ( !traversal.empty() )
  {
    // Keep dequeuing from the current level.
    if ( queuedCount > 0 )
    {
      queuedCount -= 1;
    }

    // Once we dequeued the entire level, we go down a level.
    if ( queuedCount == 0 )
    {
      queuedCount = traversal.size();
      level += 1;
    }

    const SValue* n = traversal.front();
    traversal.pop();
    f( *n, level );

    for ( const auto& child : n->children )
    {
      traversal.push( child.get() );
    }
  }
}
