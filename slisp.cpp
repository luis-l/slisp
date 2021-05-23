
#include "slisp.h"
#include "Evaluator.h"
#include "Parser.h"
#include "SValue.h"

#include <iostream>

class InteractiveEvaluator
{
public:
  void runInteractiveMode()
  {
    Environment globalEnv;
    addCoreFunctions( globalEnv );

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
          auto root = parse( input.cbegin(), input.cend() );
          out << "Evaluation Tree\n";
          out << *root << '\n';

          auto result = evaluate( globalEnv, root );
          out << result->value << '\n';

          out << "Evaluation Tree\n";
          out << *root << '\n';
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

int main()
{
  std::cout << "*hxor's LISP v0.0\n";

  InteractiveEvaluator runner;
  runner.runInteractiveMode();

  return 0;
}
