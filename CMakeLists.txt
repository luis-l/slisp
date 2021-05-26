# CMakeList.txt : CMake project for slisp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

# Add source to this project's executable.
add_executable (slisp "slisp.cpp" "slisp.h" "SValue.h" "SValue.cpp" "Parser.h" "Parser.cpp" "Evaluator.h" "Evaluator.cpp" "ListOperations.h" "ListOperations.cpp" "Environment.h" "Environment.cpp"  "Traversal.h" "Symbol.h" "Symbol.cpp" "Numeric.h")

# TODO: Add tests and install targets if needed.
