#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class SValue;

using SValueIt = std::vector< std::unique_ptr< SValue > >::iterator;

std::unique_ptr< SValue >& evaluate(
  std::unordered_map< std::string, std::unique_ptr< SValue > >& e,
  std::unique_ptr< SValue >& s );

void addCoreFunctions( std::unordered_map< std::string, std::unique_ptr< SValue > >& e );
