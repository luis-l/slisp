#pragma once

#include "Environment.h"
#include "SValueRef.h"

SValueRef evaluate( Environment& e, SValueRef s );
void addCoreFunctions( Environment& e );
