# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(mlirdb)

set(CMAKE_CXX_STANDARD 17)

# set compile commands json 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 1. Find LLVM and MLIR packages
find_package(LLVM REQUIRED CONFIG)
find_package(MLIR REQUIRED CONFIG)

message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")

# 2. Include directories
include_directories(${LLVM_INCLUDE_DIRS} ${MLIR_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include)
link_directories(${LLVM_LIBRARY_DIRS})

# include folder 
include_directories(include)

# create library 
add_library(lib 
  # parser
  lib/parser/lexer.cpp
  lib/parser/parser.cpp
  lib/parser/token.cpp
  lib/parser/ast.cpp

  # IR
  lib/IR/MLIRGen.cpp
  lib/IR/TypeConv.cpp
)

# link library to executable
add_executable(app app/main.cpp)
target_link_libraries(lib 
    PRIVATE
    MLIRIR
    MLIRFuncDialect
    MLIRArithDialect
    MLIRParser
    MLIRSupport
    LLVMSupport
)
target_link_libraries(app lib)


