
#include "Parser.h"
#include "SValue.h"

#include <regex>
#include <stack>
#include <string>

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

template < typename IteratorT >
SValueRef parse( IteratorT begin, IteratorT end )
{
  const std::string validSymbolics = "_+-*\\/=<>!&";

  auto isValidSymbol = [ &validSymbolics ]( unsigned char c ) {
    return std::isalnum( c ) || validSymbolics.find( c ) != std::string::npos;
  };

  auto isParen = []( unsigned char c ) { return c == '(' || c == ')'; };
  auto isBraces = []( unsigned char c ) { return c == '{' || c == '}'; };

  auto it = begin;

  std::stack< SValueRef > traversal;
  traversal.push( std::make_shared< SValue >( Sexpr() ) ); // Represents program

  while ( it != end )
  {
    it = std::find_if( it, end, [ isValidSymbol, isParen, isBraces ]( unsigned char c ) {
      return isParen( c ) || isBraces( c ) || isValidSymbol( c );
    } );

    if ( it != end )
    {
      const unsigned char c = *it;
      if ( c == '(' )
      {
        traversal.push( std::make_shared< SValue >( Sexpr() ) );
      }

      else if ( c == '{' )
      {
        traversal.push( std::make_shared< SValue >( QExpr() ) );
      }

      else if ( isValidSymbol( c ) )
      {
        auto contentEnd =
          std::find_if( it, end, [ isParen, isBraces ]( unsigned char c ) { return isParen( c ) || isBraces( c ); } );

        std::string line;
        std::copy( it, contentEnd, std::back_inserter( line ) );
        it = contentEnd;

        auto& parent = traversal.top();

        const auto splits = lineSplitter( line );

        for ( const std::string& s : lineSplitter( line ) )
        {
          auto child = std::make_shared< SValue >( readValue( s ) );
          parent->children.push_back( child );
        }

        continue;
      }

      else if ( c == '}' )
      {
        std::shared_ptr< SValue > top = traversal.top();
        traversal.pop();

        if ( !traversal.empty() )
        {
          traversal.top()->children.push_back( top );
        }
        else if ( !std::get_if< QExpr >( &top->value ) )
        {
          throw std::runtime_error( "Mismatch qexpr closing brace" );
        }
      }

      else if ( c == ')' )
      {
        std::shared_ptr< SValue > top = traversal.top();
        traversal.pop();

        if ( !traversal.empty() )
        {
          traversal.top()->children.push_back( top );
        }
        else if ( !std::get_if< Sexpr >( &top->value ) )
        {
          throw std::runtime_error( "Mismatched closing parens" );
        }
      }

      ++it; // next character.
    }
  }

  if ( traversal.size() != 1 ) // Only Program node should exist at the end.
  {
    throw std::runtime_error( "Mismatched parens" );
  }

  auto r = traversal.top();
  traversal.pop();
  return r;
}

using stringIt = std::string::iterator;
template std::shared_ptr< SValue > parse< stringIt >( stringIt, stringIt );

using stringConstIt = std::string::const_iterator;
template std::shared_ptr< SValue > parse< stringConstIt >( stringConstIt, stringConstIt );
