#pragma once

class SValue;
class Environment;

SValue* evalLesser( Environment& e, SValue* v );
SValue* evalLesserEqual( Environment& e, SValue* v );

SValue* evalGreater( Environment& e, SValue* v );
SValue* evalGreaterEqual( Environment& e, SValue* v );

SValue* evalEquality( Environment& e, SValue* v );
SValue* evalNotEqual( Environment& e, SValue* v );
