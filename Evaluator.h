#pragma once

#include <memory>
#include <string>
#include <vector>

class SValue;

using SValueIt = std::vector< std::unique_ptr< SValue > >::iterator;

std::unique_ptr< SValue >& evaluate( std::unique_ptr< SValue >& s );
