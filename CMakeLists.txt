# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(mlirdb)

set(CMAKE_CXX_STANDARD 17)

# set compile commands json 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# include folder 
include_directories(include)

# create library 
add_library(lib 
  lib/lib.cpp
  lib/lexer.cpp
  lib/parser.cpp
  lib/token.cpp
  lib/ast.cpp
)

# link library to executable
add_executable(app app/main.cpp)
target_link_libraries(app lib)

