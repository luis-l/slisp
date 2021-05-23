#pragma once

#include <memory>

class SValue;

/// @brief Take a Q-expression and return the Q-expression with only its first child.
std::unique_ptr< SValue >& head( std::unique_ptr< SValue >& qexpr );

/// @brief Take a Q-expression and return Q-expression without its first child.
std::unique_ptr< SValue >& tail( std::unique_ptr< SValue >& qexpr );

/// @brief Converts an S-expression to a Q-expression.
std::unique_ptr< SValue >& list( std::unique_ptr< SValue >& sexpr );

/// @brief Evaluate the Q-expression as an S-expression.
std::unique_ptr< SValue >& eval( std::unique_ptr< SValue >& qexpr );

/// @brief Concatenates multiple Q-expressions.
std::unique_ptr< SValue >& join( std::unique_ptr< SValue >& sexpr );
