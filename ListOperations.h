#pragma once

#include "SValueRef.h"

/// @brief Take a Q-expression and return the Q-expression with only its first child.
SValueRef head( SValueRef v );

/// @brief Take a Q-expression and return Q-expression without its first child.
SValueRef tail( SValueRef v );

/// @brief Converts an S-expression to a Q-expression.
SValueRef list( SValueRef v );

/// @brief Evaluate the Q-expression as an S-expression.
SValueRef eval( SValueRef v );

/// @brief Concatenates multiple Q-expressions.
SValueRef join( SValueRef v );
