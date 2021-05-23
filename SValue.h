#pragma once

#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct Sexpr
{};

struct QExpr
{};

struct Error
{
  std::string message;
};

using Value = std::variant< Sexpr, QExpr, std::string, int, Error >;

class SValue;
using Cells = std::vector< std::unique_ptr< SValue > >;

class SValue
{
public:
  SValue() = default;
  SValue( Value v ) : value( std::move( v ) )
  {}
  Value value;
  Cells children;
};

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

std::unordered_map< const SValue*, std::size_t > getDepths( const SValue& r );

std::ostream& operator<<( std::ostream& o, const Sexpr& t );
std::ostream& operator<<( std::ostream& o, const QExpr& t );
std::ostream& operator<<( std::ostream& o, const Error& e );
std::ostream& operator<<( std::ostream& o, const SValue& r );

template < typename... Args >
std::ostream& operator<<( std::ostream& o, const std::variant< Args... >& value )
{
  std::visit( [ &o ]( const auto& item ) { o << item; }, value );
  return o;
}
