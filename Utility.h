#pragma once

class SValue;
class Environment;

SValue* evalLoad( Environment& e, SValue* v );
SValue* evalPrint( Environment& e, SValue* v );
SValue* evalError( Environment& e, SValue* v );
SValue* evalShow( Environment& e, SValue* v );
