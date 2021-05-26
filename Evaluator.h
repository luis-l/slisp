#pragma once

#include "Environment.h"

SValue* evaluate( Environment& e, SValue* s );
void addCoreFunctions( Environment& e );
