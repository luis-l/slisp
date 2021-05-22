
#include "slisp.h"

#include <assert.h>
#include <functional>
#include <iostream>
#include <list>
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

  using SValueIt = std::vector< std::unique_ptr< SValue > >::iterator;
  using SValueConstIt = std::vector< std::unique_ptr< SValue > >::const_iterator;

  using Root = std::unique_ptr< SValue >;

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

  std::unordered_map< const SValue*, std::size_t > getDepths( const Root& r )
  {
    std::unordered_map< const SValue*, std::size_t > depths;
    std::queue< const SValue* > traversal;
    traversal.push( r.get() );
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

  std::ostream& operator<<( std::ostream& o, const Root& r )
  {
    std::stack< const SValue* > traversal;
    const std::unordered_map< const SValue*, std::size_t > depths = getDepths( r );
    traversal.push( r.get() );
    while ( !traversal.empty() )
    {
      const SValue* n = traversal.top();
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
  Root parse( IteratorT begin, IteratorT end )
  {
    const std::string validSymbolics = "_+-*\\/=<>!&";

    auto isValidSymbol = [ &validSymbolics ]( unsigned char c ) {
      return std::isalnum( c ) || validSymbolics.find( c ) != std::string::npos;
    };

    auto isParen = []( unsigned char c ) { return c == '(' || c == ')'; };

    auto it = begin;

    std::stack< std::unique_ptr< SValue > > traversal;
    traversal.push( std::make_unique< SValue >( ExpressionType() ) ); // Represents program

    while ( it != end )
    {
      it = std::find_if(
        it, end, [ isValidSymbol, isParen ]( unsigned char c ) { return isParen( c ) || isValidSymbol( c ); } );

      if ( it != end )
      {
        const unsigned char c = *it;
        if ( c == '(' )
        {
          traversal.push( std::make_unique< SValue >( ExpressionType() ) );
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
            auto child = std::make_unique< SValue >( readValue( s ) );
            parent->children.push_back( std::move( child ) );
          }

          continue;
        }

        else if ( c == ')' )
        {
          std::unique_ptr< SValue > top = std::move( traversal.top() );
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

    if ( traversal.size() != 1 ) // Only Program node should exist at the end.
    {
      throw std::runtime_error( "Mismatched parens" );
    }

    Root r = std::move( traversal.top() );
    traversal.pop();
    return r;
  }

  std::unique_ptr< SValue >& evaluate( std::unique_ptr< SValue >& s );

  std::unique_ptr< SValue >& evalOp( const std::string& op, SValueIt begin, SValueIt end )
  {
    const bool allIntegral = std::all_of(
      begin, end, []( const std::unique_ptr< SValue >& s ) { return std::get_if< int >( &s->value ) != nullptr; } );

    if ( !allIntegral )
    {
      ( *begin )->value = Error( "Not all arguments are integral" );
      return ( *begin );
    }

    // Negation
    if ( op == "-" && ( end - begin == 1 ) )
    {
      ( *begin )->value = -std::get< int >( ( *begin )->value );
      return ( *begin );
    }

    auto integralOperator = [ &op ]() -> std::function< Value( const Value&, const std::unique_ptr< SValue >& ) > {
      if ( op == "+" )
      {
        return []( const Value& x, const std::unique_ptr< SValue >& y ) {
          return Value( std::get< int >( x ) + std::get< int >( y->value ) );
        };
      }
      if ( op == "-" )
      {
        return []( const Value& x, const std::unique_ptr< SValue >& y ) {
          return Value( std::get< int >( x ) - std::get< int >( y->value ) );
        };
      }
      if ( op == "*" )
      {
        return []( const Value& x, const std::unique_ptr< SValue >& y ) {
          return Value( std::get< int >( x ) * std::get< int >( y->value ) );
        };
      }
      if ( op == "/" )
      {
        return []( const Value& x, const std::unique_ptr< SValue >& y ) {
          if ( std::get_if< Error >( &x ) )
          {
            return x; // Propagate error.
          }
          int yint = std::get< int >( y->value );
          return yint == 0 ? Value( Error( "Division by zero" ) ) : Value( std::get< int >( x ) / yint );
        };
      }

      return []( const Value& x, const std::unique_ptr< SValue >& y ) { return Value( Error( "Unknown operator" ) ); };
    };

    ( *begin )->value = std::accumulate( begin + 1, end, ( *begin )->value, integralOperator() );
    return *begin;
  }

  std::unique_ptr< SValue >& evaluateSexpr( std::unique_ptr< SValue >& s )
  {
    for ( auto& child : s->children )
    {
      child->value = evaluate( child )->value;
      child->children.clear(); // Done with this - memory can be freed.
    }

    // Atom.
    if ( s->children.empty() )
    {
      return s;
    }

    // nested superfluous expressions. e.g. ( (+ 1 2) )
    if ( s->children.size() == 1 )
    {
      return s->children.front();
    }

    if ( auto op = std::get_if< std::string >( &s->children.front()->value ) )
    {
      return evalOp( *op, s->children.begin() + 1, s->children.end() );
    }
    else
    {
      s->value = Error( "Unsupported operator" );
      return s;
    }
  }

  std::unique_ptr< SValue >& evaluate( std::unique_ptr< SValue >& s )
  {
    if ( std::get_if< ExpressionType >( &s->value ) )
    {
      return evaluateSexpr( s );
    }

    return s;
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
            Root root = parse( input.cbegin(), input.cend() );
            out << root << '\n';

            auto& result = evaluate( root );
            out << result->value << '\n';

            out << root << '\n';
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
