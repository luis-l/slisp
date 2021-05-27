
#include "Evaluator.h"
#include "ListOperations.h"
#include "Numeric.h"
#include "Ordering.h"
#include "SValue.h"
#include "Utility.h"

#include <type_traits>

SValue* evaluateSexpr( Environment& e, SValue* s );
SValue* evaluateNumeric( const std::string& op, SValue* v );
SValue* evaluateDef( Environment& e, SValue* v );
SValue* evaluateLambda( Environment& e, SValue* v );
SValue* invokeLambda( Lambda&, Environment& e, SValue* v );
SValue* evalQexpr( Environment& e, SValue* v );

SValue* evalConditional( Environment& e, SValue* v );

const Symbol plusSymbol( "+" );
const Symbol minusSymbol( "-" );
const Symbol multSymbol( "*" );
const Symbol divSymbol( "/" );

const Symbol headSymbol( "head" );
const Symbol tailSymbol( "tail" );
const Symbol listSymbol( "list" );
const Symbol joinSymbol( "join" );
const Symbol evalSymbol( "eval" );

const Symbol defSymbol( "def" );
const Symbol lambdaSymbol( "\\" );

const Symbol lesserSymbol( "<" );
const Symbol lesserEqualSymbol( "<=" );

const Symbol greaterSymbol( ">" );
const Symbol greaterEqualSymbol( ">=" );

const Symbol equalSymbol( "eq" );
const Symbol notEqualSymbol( "neq" );

const Symbol conditionalSymbol( "if" );

const Symbol loadSymbol( "load" );
const Symbol printSymbol( "print" );
const Symbol errorSymbol( "error" );

void addCoreFunctions( Environment& e )
{
  auto bindNumericOp = []( const std::string& op ) -> CoreFunction {
    return [ op ]( Environment&, SValue* v ) -> SValue* { return evaluateNumeric( op, v ); };
  };

  e.set( plusSymbol, SValue( bindNumericOp( "+" ) ) );
  e.set( minusSymbol, SValue( bindNumericOp( "-" ) ) );
  e.set( multSymbol, SValue( bindNumericOp( "*" ) ) );
  e.set( divSymbol, SValue( bindNumericOp( "/" ) ) );

  auto bindListOp = []( auto f ) { return [ f ]( Environment&, SValue* v ) -> SValue* { return f( v ); }; };

  e.set( headSymbol, SValue( bindListOp( head ) ) );
  e.set( tailSymbol, SValue( bindListOp( tail ) ) );
  e.set( listSymbol, SValue( bindListOp( list ) ) );
  e.set( joinSymbol, SValue( bindListOp( join ) ) );
  e.set( evalSymbol, SValue( []( Environment& e, SValue* v ) -> SValue* { return evalQexpr( e, v ); } ) );
  e.set( defSymbol, SValue( evaluateDef ) );
  e.set( lambdaSymbol, SValue( evaluateLambda ) );

  e.set( lesserSymbol, SValue( evalLesser ) );
  e.set( lesserEqualSymbol, SValue( evalLesserEqual ) );

  e.set( greaterSymbol, SValue( evalGreater ) );
  e.set( greaterEqualSymbol, SValue( evalGreaterEqual ) );

  e.set( equalSymbol, SValue( evalEquality ) );
  e.set( notEqualSymbol, SValue( evalNotEqual ) );

  e.set( conditionalSymbol, SValue( evalConditional ) );

  e.set( loadSymbol, SValue( evalLoad ) );
  e.set( printSymbol, SValue( evalPrint ) );
  e.set( errorSymbol, SValue( evalError ) );
}

SValue* evaluate( Environment& e, SValue* v )
{
  // Symbol
  if ( auto symbol = v->getIf< Symbol >() )
  {
    v = e.get( *symbol, v );
  }

  if ( v->isSExpression() )
  {
    v = evaluateSexpr( e, v );
  }

  return v;
}

SValue* evaluateSexpr( Environment& e, SValue* s )
{
  s->foreachCell( [ &e ]( SValue& child ) { std::swap( child, *evaluate( e, &child ) ); } );

  // Atom.
  if ( s->isEmpty() )
  {
    return s;
  }

  if ( s->size() == 1 )
  {
    Cells& cells = s->cellsRequired();
    std::unique_ptr< SValue > a = cells.takeFront();
    std::swap( *s, *evaluate( e, a.get() ) );
    return s;
  }

  Cells& cells = s->cellsRequired();
  std::unique_ptr< SValue > operation = cells.takeFront();

  if ( auto callable = operation->getIf< CoreFunction >() )
  {
    return ( *callable )( e, s );
  }
  else if ( auto l = operation->getIf< Lambda >() )
  {
    return invokeLambda( *l, e, s );
  }
  else if ( operation->isSExpression() && operation->isEmpty() )
  { // Ignore Empty S-expression
    // Special case feature. Allow multiple definitions within an S-expression.
    // e.g.  (def {a} 10) (def {b} 20 ) ( def {c} 30 ) ==> ()
    return evaluate( e, s ); // Evaluate the rest
  }
  else
  {
    return error( s, "Operation is not callable" );
  }
}

SValue* invokeLambda( Lambda& l, Environment& e, SValue* s )
{
  Cells& formalCells = l.formals->cellsRequired();
  Cells& argCells = s->cellsRequired();

  // Bind all arguments.
  while ( !argCells.isEmpty() )
  {
    REQUIRE( s, !formalCells.isEmpty(), "Passed to many arguments to function" );

    std::unique_ptr< SValue > formalArg = formalCells.takeFront();

    const Symbol& sym = formalArg->get< Symbol >();

    // Special case for variadics.
    if ( sym.label == "&" )
    {
      std::unique_ptr< SValue > restOfArgsFormal = formalCells.takeFront();
      REQUIRE( s, formalCells.isEmpty(), "There should only be 1 symbol after &" );

      // We need to bind last formal to the remaining input arguments as a Q-expression.
      l.env.set( restOfArgsFormal->get< Symbol >(), *list( s ) );
      break;
    }

    std::unique_ptr< SValue > realArg = argCells.takeFront();
    l.env.set( sym, *realArg );
  }

  // No arguments passed for variadic. e.g. + 1
  if ( !formalCells.isEmpty() && formalCells.front()->get< Symbol >().label == "&" )
  {
    std::unique_ptr< SValue > formalArg = formalCells.takeFront(); // Remove &

    REQUIRE( s, formalCells.size() == 1, "There should only be 1 symbol after &" );
    std::unique_ptr< SValue > restOfArgsFormal = formalCells.takeFront();

    // Bind the formal to an empty Q-expression.
    l.env.set( restOfArgsFormal->get< Symbol >(), SValue( QExpr() ) );
  }

  // Do full function application, every formal argument is now bound.
  if ( formalCells.isEmpty() )
  {
    l.env.parent = &e;
    // Make an S-expression and add a lambda copy for evaluation.
    // The S-expression is so eval can evaluate it.
    Cells c;
    c.append( makeSValue( l.body->value ) );
    s->value = std::move( c );
    std::swap( *s, *evalQexpr( l.env, s ) );
    return s;
  }

  else
  {
    // Partial application, return new lambda
    // argCount < formalCount
    s->value = Lambda( l );
    return s;
  }
}

SValue* evaluateLambda( Environment& e, SValue* v )
{
  REQUIRE( v, v->size() == 2, "lambda requires 2 arguments" );

  Cells& cells = v->cellsRequired();
  std::unique_ptr< SValue > formals = cells.takeFront();
  std::unique_ptr< SValue > body = cells.takeFront();

  REQUIRE( v, formals->isType< QExpr >(), "First lambda argument must be Q-expression" );
  REQUIRE( v, body->isType< QExpr >(), "Second lambda argument must be Q-expression" );

  Cells& formalCells = formals->cellsRequired();
  const bool allFormalsAreSymbols =
    std::all_of( formalCells.begin(), formalCells.end(), []( const auto& c ) { return c->isType< Symbol >(); } );

  REQUIRE( v, allFormalsAreSymbols, "Lambda formals can only contains Symbols" );

  // Create the lambda.
  v->value = Lambda( Environment(), std::move( formals ), std::move( body ) );
  return v;
}

SValue* evaluateDef( Environment& e, SValue* v )
{
  REQUIRE( v, v->size() >= 2, "def requires 2 or more arguments" );

  Cells& cells = v->cellsRequired();

  // def { x y }   10 20
  //      symbols  expressions
  // where it will bind: x = 10, y = 20

  std::unique_ptr< SValue > symbols = cells.takeFront();
  Cells& symbolCells = symbols->cellsRequired();

  REQUIRE( v, symbols->isQExpression(), "def must take a Q-expression for the first argument" );
  REQUIRE( v, !symbolCells.isEmpty(), "def requires a non-empty Q-expression for first argument" );
  REQUIRE( v, symbolCells.size() == cells.size(), "Symbol count must match expression count" );

  const bool allSymbolsAreSymbolType =
    std::all_of( symbolCells.begin(), symbolCells.end(), []( const auto& c ) { return c->isType< Symbol >(); } );

  REQUIRE( v, allSymbolsAreSymbolType, "Cannot define for non-symbol type" );

  for ( std::size_t i = 0; i < symbolCells.size(); ++i )
  {
    e.rootSet( symbolCells[ i ]->get< Symbol >(), *cells[ i ] );
  }

  return empty( v );
}

SValue* evaluateNumeric( const std::string& op, SValue* v )
{
  Cells& cells = v->cellsRequired();
  if ( cells.front()->isType< int >() )
  {
    return evaluateNumericT< int >( op, v );
  }

  // Fall back to float.
  return evaluateNumericT< double >( op, v );
}

/// @brief Evaluate the Q-expression as an S-expression.
SValue* evalQexpr( Environment& e, SValue* v )
{
  Cells& args = v->cellsRequired();
  REQUIRE( v, args.size() == 1, "eval requires 1 argument" );

  SValue* qexpr = args.front();
  REQUIRE( v, qexpr->isQExpression(), "eval expects a QExpression" );

  Cells& qexprCells = qexpr->cellsRequired();

  // Move the Q-expression cells into an S-expression.
  v->value = Cells{ std::move( qexprCells ) };
  return evaluate( e, v );
}

SValue* evalConditional( Environment& e, SValue* v )
{
  Cells& cells = v->cellsRequired();
  std::unique_ptr< SValue > condition = cells.takeFront();
  REQUIRE( v, condition->isType< Boolean >(), "if expects boolean as first argument" );

  std::unique_ptr< SValue > first = cells.takeFront();
  REQUIRE( v, first->isQExpression(), "if expects Q-expression as second argument" );

  std::unique_ptr< SValue > second = cells.takeFront();
  REQUIRE( v, second->isQExpression(), "if expects Q-expression as third argument" );

  if ( condition->get< Boolean >() == Boolean::True )
  {
    // Make the first argument to an S-expression so it can be evaulated.
    v->value = Cells( std::move( first->cellsRequired() ) );
    return evaluate( e, v );
  }
  else
  {
    // Make the second argument to an S-expression so it can be evaulated.
    v->value = Cells( std::move( second->cellsRequired() ) );
    return evaluate( e, v );
  }
}
