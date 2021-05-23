#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class SValue;

std::shared_ptr< SValue > evaluate(
  std::unordered_map< std::string, std::shared_ptr< SValue > >& e,
  std::shared_ptr< SValue > s );

void addCoreFunctions( std::unordered_map< std::string, std::shared_ptr< SValue > >& e );
