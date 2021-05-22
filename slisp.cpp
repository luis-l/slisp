
#include "slisp.h"

#include <assert.h>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <queue>
#include <regex>
#include <stack>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace slisp
{
  const std::regex integerRegex( "-?[0-9]+" );

  /// Strict float regex. Only matches 0.0 format.
  /// 0. and .0 are not supported.
  const std::regex floatRegex( "-?[0-9]+[.][0-9]+" );

  // + 1 2 3 4
  // + (* 1 2 3) (- 10 10)
  // op arg+                       zero or more arguments
  // exp: atom | op exp+

  bool isInteger( const std::string& s )
  {
    return std::regex_match( s, integerRegex );
  }

  bool isFloat( const std::string& s )
  {
    return std::regex_match( s, floatRegex );
  }

  std::vector< std::string > lineSplitter( const std::string& line )
  {
    auto isWhitespace = []( unsigned char c ) { return std::isspace( c ); };

    // Find the start index of a word in the line, starting from pos.
    auto findWordStart = [ &line, isWhitespace ]( const std::size_t pos ) {
      auto it = std::find_if_not( line.cbegin() + pos, line.cend(), isWhitespace );
      return it == line.cend() ? line.size() : it - line.cbegin();
    };

    // Find the end index of a word in the line, starting from pos.
    auto findWordEnd = [ &line, isWhitespace ]( const std::size_t pos ) {
      auto it = std::find_if( line.cbegin() + pos, line.cend(), isWhitespace );
      return it == line.cend() ? line.size() : it - line.cbegin();
    };

    std::vector< std::string > splits;

    // Start is the starting position of a word.
    constexpr std::size_t lineStart = 0;
    std::size_t start = findWordStart( lineStart );

    while ( start < line.size() )
    {
      // End is the ending position of a word.
      const std::size_t end = findWordEnd( start );
      splits.push_back( line.substr( start, ( end - start ) ) );
      start = findWordStart( end );
    }

    return splits;
  }

  struct ExpressionType
  {};

  std::ostream& operator<<( std::ostream& o, const ExpressionType& t )
  {
    return o << "expr";
  }

  struct Error
  {
    std::string message;
  };

  std::ostream& operator<<( std::ostream& o, const Error& e )
  {
    return o << e.message;
  }

  using Value = std::variant< ExpressionType, std::string, int, Error >;

  class SyntaxTree
  {
  public:
    class Node
    {
    public:
      Node() = default;
      Node( Value v ) : value( std::move( v ) )
      {}
      Value value;
      std::vector< std::unique_ptr< Node > > children;
    };

    std::unique_ptr< Node > root;
  };

  Value readValue( const std::string& text )
  {
    if ( isInteger( text ) )
    {
      return std::stoi( text );
    }
    else
    {
      return text;
    }
  }

  std::unordered_map< const SyntaxTree::Node*, std::size_t > getDepths( const SyntaxTree& t )
  {
    std::unordered_map< const SyntaxTree::Node*, std::size_t > depths;
    std::queue< const SyntaxTree::Node* > traversal;
    traversal.push( t.root.get() );
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

      const SyntaxTree::Node* n = traversal.front();
      traversal.pop();
      depths[ n ] = level;

      for ( const auto& child : n->children )
      {
        traversal.push( child.get() );
      }
    }
    return depths;
  }

  template < typename... Args >
  std::ostream& operator<<( std::ostream& o, const std::variant< Args... >& value )
  {
    std::visit( [ &o ]( const auto& item ) { o << item; }, value );
    return o;
  }

  std::ostream& operator<<( std::ostream& o, const SyntaxTree& t )
  {
    std::stack< const SyntaxTree::Node* > traversal;
    const std::unordered_map< const SyntaxTree::Node*, std::size_t > depths = getDepths( t );
    traversal.push( t.root.get() );
    while ( !traversal.empty() )
    {
      const SyntaxTree::Node* n = traversal.top();
      traversal.pop();

      const std::string padding( depths.at( n ) * 2, ' ' );
      o << padding << "value: '" << n->value << "'\n";
      for ( const auto& child : n->children )
      {
        traversal.push( child.get() );
      }
    }

    return o;
  }

  template < typename IteratorT >
  SyntaxTree parse( IteratorT begin, IteratorT end )
  {
    const std::string validSymbolics = "_+-*\\/=<>!&";

    auto isValidSymbol = [ &validSymbolics ]( unsigned char c ) {
      return std::isalnum( c ) || validSymbolics.find( c ) != std::string::npos;
    };

    auto isParen = []( unsigned char c ) { return c == '(' || c == ')'; };

    auto it = begin;

    std::stack< std::unique_ptr< SyntaxTree::Node > > traversal;
    traversal.push( std::make_unique< SyntaxTree::Node >( ExpressionType() ) ); // Represents program

    while ( it != end )
    {
      it = std::find_if(
        it, end, [ isValidSymbol, isParen ]( unsigned char c ) { return isParen( c ) || isValidSymbol( c ); } );

      if ( it != end )
      {
        const unsigned char c = *it;
        if ( c == '(' )
        {
          traversal.push( std::make_unique< SyntaxTree::Node >( ExpressionType() ) );
        }

        else if ( isValidSymbol( c ) )
        {
          auto contentEnd = std::find_if( it, end, isParen );
          std::string line;
          std::copy( it, contentEnd, std::back_inserter( line ) );
          it = contentEnd;

          auto& parent = traversal.top();

          const auto splits = lineSplitter( line );

          for ( const std::string& s : lineSplitter( line ) )
          {
            auto child = std::make_unique< SyntaxTree::Node >( readValue( s ) );
            parent->children.push_back( std::move( child ) );
          }

          continue;
        }

        else if ( c == ')' )
        {
          std::unique_ptr< SyntaxTree::Node > top = std::move( traversal.top() );
          traversal.pop();

          if ( !traversal.empty() )
          {
            traversal.top()->children.push_back( std::move( top ) );
          }
          else
          {
            throw std::runtime_error( "Extra closing paren" );
          }
        }

        ++it; // next character.
      }
    }

    SyntaxTree t;

    if ( traversal.size() != 1 ) // Only Program node should exist at the end.
    {
      throw std::runtime_error( "Mismatched parens" );
    }

    t.root = std::move( traversal.top() );
    traversal.pop();
    return t;
  }

  Value evalOp( const std::string& op, const Value& x, const Value& y )
  {
    auto evalIntegral = [ &x, &y ]( auto opFunctor ) -> Value {
      auto xval = std::get_if< int >( &x );
      auto yval = std::get_if< int >( &y );
      return xval && yval ? Value( opFunctor( *xval, *yval ) ) : Value( Error( "Arguments are not integral" ) );
    };

    if ( op == "+" )
    {
      return evalIntegral( std::plus< int >() );
    }
    if ( op == "-" )
    {
      return evalIntegral( std::minus< int >() );
    }
    if ( op == "*" )
    {
      return evalIntegral( std::multiplies< int >() );
    }
    if ( op == "/" )
    {
      auto xval = std::get_if< int >( &x );
      auto yval = std::get_if< int >( &y );
      if ( xval && yval )
      {
        return *yval != 0 ? Value( *xval / *yval ) : Value( Error( "Division by zero" ) );
      }
      return Value( Error( "Arguments are not integral" ) );
    }

    throw std::runtime_error( "Unsupported operator" );
    return 0;
  }

  Value evaluate( const SyntaxTree::Node* node )
  {
    // Atom.
    if ( node->children.empty() )
    {
      return node->value;
    }

    // nested superfluous expressions. e.g. ( (+ 1 2) )
    if ( node->children.size() == 1 )
    {
      // Should only occur in expression nesting.
      assert( std::get_if< ExpressionType >( &node->value ) );

      // Go next level.
      return evaluate( node->children.front().get() );
    }

    const auto& op = std::get< std::string >( node->children[ 0 ]->value );

    // Evaluate first argument.
    Value result = evaluate( node->children[ 1 ].get() );

    // Evaluate the rest of the arguments.
    for ( int i = 2; i < node->children.size(); ++i )
    {
      result = evalOp( op, result, evaluate( node->children[ i ].get() ) );
    }

    return result;
  }

  class InteractiveEvaluator
  {
  public:
    void runInteractiveMode()
    {
      out << std::boolalpha;

      bool isDone = false;
      while ( !isDone )
      {
        out << ">> ";
        std::string input;
        std::getline( in, input );

        if ( input == "exit" )
        {
          isDone = true;
          out << "Exiting...\n";
        }
        else
        {
          try
          {
            SyntaxTree t = parse( input.cbegin(), input.cend() );
            //out << t << '\n';
            out << evaluate( t.root.get() ) << '\n';
          }
          catch ( const std::exception& e )
          {
            out << e.what() << '\n';
          }
        }
      }
    }

    std::ostream& out = std::cout;
    std::istream& in = std::cin;
  };

}

int main()
{
  std::cout << "*Shrek LISP v0.0\n";

  slisp::InteractiveEvaluator runner;
  runner.runInteractiveMode();

  return 0;
}
