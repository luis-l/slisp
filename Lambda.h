#pragma once

#include "Environment.h"

#include <memory>

class SValue;

// \  { x y }         {+ x y}
//    formals qexpr   body qexpr
struct Lambda
{
  Lambda();
  Lambda( Environment e, std::unique_ptr< SValue > formals, std::unique_ptr< SValue > body );

  Lambda( const Lambda& other );
  Lambda& operator=( const Lambda& other );

  Lambda( Lambda&& other ) noexcept;
  Lambda& operator=( Lambda&& other ) noexcept;

  Environment env;
  std::unique_ptr< SValue > formals;
  std::unique_ptr< SValue > body;
};
