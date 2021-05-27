
#include "slisp.h"
#include "Evaluator.h"
#include "Parser.h"
#include "SValue.h"
#include "Utility.h"

#include <iostream>

Environment createDefaultEnvironment()
{
  Environment env;
  addCoreFunctions( env );
  return env;
}

class InteractiveEvaluator
{
public:
  void runInteractiveMode()
  {
    Environment globalEnv = createDefaultEnvironment();

    {
      auto root = makeDefaultSValue();
      root->cells()->append( makeSValue( "standard/Standard.slisp" ) );
      SValue* result = evalLoad( globalEnv, root.get() );
      if ( result->isError() )
      {
        out << *result << '\n';
      }
    }

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
      else if ( input == "env" )
      {
        out << globalEnv << '\n';
      }
      else
      {
        try
        {
          auto root = parse( input.cbegin(), input.cend() );
          out << "ast: ";
          show( out, *root ) << '\n';

          auto result = evaluate( globalEnv, root.get() );
          out << "==> ";
          show( out, *result ) << "\n\n";
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

int main( int argc, char** argv )
{
  if ( argc >= 2 )
  {
    Environment globalEnv = createDefaultEnvironment();

    const std::string filename( argv[ 1 ] );
    auto root = makeDefaultSValue();
    root->cells()->append( makeSValue( filename ) );

    SValue* result = evalLoad( globalEnv, root.get() );
    std::cout << *result << '\n';
  }
  else
  {
    std::cout << "*hxor's LISP v0.0\n";
    InteractiveEvaluator runner;
    runner.runInteractiveMode();
  }

  return 0;
}
