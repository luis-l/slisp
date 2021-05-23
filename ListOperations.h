#pragma once

#include <memory>

class SValue;

/// @brief Take a Q-expression and return the Q-expression with only its first child.
std::shared_ptr< SValue > head( std::shared_ptr< SValue > v );

/// @brief Take a Q-expression and return Q-expression without its first child.
std::shared_ptr< SValue > tail( std::shared_ptr< SValue > v );

/// @brief Converts an S-expression to a Q-expression.
std::shared_ptr< SValue > list( std::shared_ptr< SValue > v );

/// @brief Evaluate the Q-expression as an S-expression.
std::shared_ptr< SValue > eval( std::shared_ptr< SValue > v );

/// @brief Concatenates multiple Q-expressions.
std::shared_ptr< SValue > join( std::shared_ptr< SValue > v );
