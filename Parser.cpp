
#include "Parser.h"
#include "SValue.h"

#include <regex>
#include <stack>
#include <string>

const std::regex integerRegex( "-?[0-9]+" );

/// Strict float regex. Only matches 0.0 format.
/// 0. and .0 are not supported.
const std::regex floatRegex( "-?[0-9]+[.][0-9]+" );

const std::regex stringLiteralRegex( R"("(\\.|[^"])*")" );
//const std::regex stringLiteralRegex( "\"(\\\\.|[^\"])*\"" );

bool isBool( const std::string& s )
{
  return s == "true" || s == "false";
}

bool isInteger( const std::string& s )
{
  return std::regex_match( s, integerRegex );
}

bool isFloat( const std::string& s )
{
  return std::regex_match( s, floatRegex );
}

// String literals is quoted text.
bool isStringLiteral( const std::string& s )
{
  return std::regex_match( s, stringLiteralRegex );
}

Value readValue( const std::string& text )
{
  if ( isBool( text ) )
  {
    return Boolean( text == "true" ? Boolean::True : Boolean::False );
  }
  else if ( isInteger( text ) )
  {
    return std::stoi( text );
  }
  else if ( isFloat( text ) )
  {
    return std::stod( text );
  }
  else
  {
    return Symbol( text );
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
std::unique_ptr< SValue > parse( IteratorT begin, IteratorT end )
{
  const std::string validSymbolics = "_+-*\\/=<>!&";

  auto isValidSymbol = [ &validSymbolics ]( unsigned char c ) {
    return std::isalnum( c ) || validSymbolics.find( c ) != std::string::npos;
  };

  auto isParen = []( unsigned char c ) { return c == '(' || c == ')'; };
  auto isBraces = []( unsigned char c ) { return c == '{' || c == '}'; };

  auto it = begin;

  // Root for the entire program
  std::unique_ptr< SValue > root = makeDefaultSValue();

  std::stack< SValue* > traversal;
  traversal.push( root.get() );

  auto appendCellOnly = [ &traversal ]( auto&& value ) -> SValue* {
    SValue* parent = traversal.top();
    Cells& cells = parent->cellsRequired();
    cells.append( makeSValue( std::forward< decltype( value ) >( value ) ) );
    return cells.back();
  };

  auto appendAndTraverseCell = [ &traversal, &appendCellOnly ]( auto&& value ) {
    traversal.push( appendCellOnly( std::forward< decltype( value ) >( value ) ) );
  };

  auto popCell = [ &traversal ]() -> SValue* {
    SValue* s = traversal.top();
    traversal.pop();
    return s;
  };

  while ( it != end )
  {
    it = std::find_if( it, end, [ isValidSymbol, isParen, isBraces ]( unsigned char c ) {
      return isParen( c ) || isBraces( c ) || isValidSymbol( c ) || c == '"';
    } );

    if ( it != end )
    {
      const unsigned char c = *it;

      // begin String literal
      if ( c == '"' )
      {
        std::smatch stringMatch;

        // regex search does not like temporary strings. Cannot seemd to get iterators to work with it.
        std::string text( begin, end );

        const bool stringFound = std::regex_search( text, stringMatch, stringLiteralRegex );
        if ( stringFound )
        {
          const std::string stringText = stringMatch.str();
          appendCellOnly( stringText );
          const auto offset = stringMatch.position() + stringMatch.length();
          it += offset;
          continue;
        }
        else
        {
          throw std::runtime_error( "Missing string literal quote" );
        }
      }

      else if ( c == '(' )
      {
        appendAndTraverseCell( Cells() );
      }

      else if ( c == '{' )
      {
        appendAndTraverseCell( QExpr() );
      }

      else if ( isValidSymbol( c ) )
      {
        auto contentEnd =
          std::find_if( it, end, [ isParen, isBraces ]( unsigned char c ) { return isParen( c ) || isBraces( c ); } );

        std::string line;
        std::copy( it, contentEnd, std::back_inserter( line ) );
        it = contentEnd;

        const auto splits = lineSplitter( line );
        for ( const std::string& s : lineSplitter( line ) )
        {
          appendCellOnly( readValue( s ) );
        }

        continue;
      }

      else if ( c == '}' )
      {
        SValue* s = popCell();
        if ( traversal.empty() && !s->isQExpression() )
        {
          throw std::runtime_error( "Mismatch Q-expression closing brace" );
        }
      }

      else if ( c == ')' )
      {
        SValue* s = popCell();
        if ( traversal.empty() && !s->isSExpression() )
        {
          throw std::runtime_error( "Mismatch S-expression closing brace" );
        }
      }

      ++it; // next character.
    }
  }

  if ( traversal.size() != 1 ) // Only Program node should exist at the end.
  {
    throw std::runtime_error( "Mismatched parentheses" );
  }

  popCell();
  return root;
}

using stringIt = std::string::iterator;
template std::unique_ptr< SValue > parse< stringIt >( stringIt, stringIt );

using stringConstIt = std::string::const_iterator;
template std::unique_ptr< SValue > parse< stringConstIt >( stringConstIt, stringConstIt );
