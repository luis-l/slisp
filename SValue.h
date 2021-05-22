#pragma once

#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct ExpressionType
{};

struct Error
{
  std::string message;
};

using Value = std::variant< ExpressionType, std::string, int, Error >;

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

  template < typename UnaryOp >
  void traversePreorder( UnaryOp f ) const
  {
    std::stack< const SValue* > traversal;
    traversal.push( this );
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
  void traverseLevelOrder( BinaryOp f )
  {
    std::queue< const SValue* > traversal;
    traversal.push( this );
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
};

std::unordered_map< const SValue*, std::size_t > getDepths( const std::unique_ptr< SValue >& r );

std::ostream& operator<<( std::ostream& o, const ExpressionType& t );
std::ostream& operator<<( std::ostream& o, const Error& e );
std::ostream& operator<<( std::ostream& o, const std::unique_ptr< SValue >& r );

template < typename... Args >
std::ostream& operator<<( std::ostream& o, const std::variant< Args... >& value )
{
  std::visit( [ &o ]( const auto& item ) { o << item; }, value );
  return o;
}
