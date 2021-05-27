# CMakeList.txt : CMake project for slisp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

# Add source to this project's executable.
add_executable (
  slisp 
  "slisp.cpp" 
  "slisp.h" 
  "Cells.cpp" 
  "Cells.h" 
  "Environment.cpp"  
  "Environment.h" 
  "Evaluator.cpp" 
  "Evaluator.h" 
  "Lambda.cpp" 
  "Lambda.h" 
  "ListOperations.cpp" 
  "ListOperations.h" 
  "Numeric.h" 
  "Ordering.cpp"
  "Ordering.h" 
  "Parser.cpp" 
  "Parser.h" 
  "SValue.cpp" 
  "SValue.h" 
  "Symbol.cpp" 
  "Symbol.h" 
  "Traversal.h" 
  "Utility.cpp"
  "Utility.h" )

# Set start up project for VS
set_property(
  DIRECTORY 
  ${CMAKE_CURRENT_SOURCE_DIR}
  PROPERTY VS_STARTUP_PROJECT slisp )

# Copy standard library
add_custom_command( 
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_CURRENT_SOURCE_DIR}/standard
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/standard )

# TODO: Add tests and install targets if needed.
