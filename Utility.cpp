
#include "Utility.h"

#include "Evaluator.h"
#include "Parser.h"
#include "SValue.h"

#include <fstream>
#include <iostream>

// v contains the string path
SValue* evalLoad( Environment& e, SValue* v )
{
  REQUIRE( v, v->size() == 1, "load requires one argument" );

  Cells& cells = v->cellsRequired();

  std::unique_ptr< SValue > file = cells.takeFront();
  REQUIRE( v, file->isType< std::string >(), "load requires a string argument" );

  std::ifstream reader( file->get< std::string >() );
  if ( !reader.good() )
  {
    return error( v, "Could not read file" );
  }

  std::string text( ( std::istreambuf_iterator< char >( reader ) ), std::istreambuf_iterator< char >() );
  std::unique_ptr< SValue > script = parse( text.cbegin(), text.cend() );
  Cells& scriptExpressions = script->cellsRequired();
  while ( !scriptExpressions.isEmpty() )
  {
    std::unique_ptr< SValue > v = scriptExpressions.takeFront();
    SValue* result = evaluate( e, v.get() );
    if ( result->isError() )
    {
      std::cout << *result << '\n';
    }
  }
  return empty( v );
}

SValue* evalPrint( Environment& e, SValue* v )
{
  v->foreachCell( []( const SValue& v ) { show( std::cout, v ) << ' '; } );
  std::cout << '\n';
  return empty( v );
}

SValue* evalError( Environment& e, SValue* v )
{
  REQUIRE( v, v->size() == 1, "error requires one argument" );

  Cells& cells = v->cellsRequired();

  std::unique_ptr< SValue > errorMessage = cells.takeFront();
  REQUIRE( v, errorMessage->isType< std::string >(), "error requires a string argument" );

  return error( v, errorMessage->get< std::string >() );
}