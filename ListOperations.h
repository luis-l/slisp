#pragma once

#include <memory>

class SValue;

/// @brief Take a Q-expression and return the Q-expression with only its first child.
SValue* head( SValue* v );

/// @brief Take a Q-expression and return Q-expression without its first child.
SValue* tail( SValue* v );

/// @brief Converts an S-expression to a Q-expression.
SValue* list( SValue* v );

/// @brief Concatenates multiple Q-expressions.
SValue* join( SValue* v );
