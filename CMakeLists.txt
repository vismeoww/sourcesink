# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(sourcesink)

set(CMAKE_CXX_STANDARD 17)

# set compile commands json 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 1. Find LLVM and MLIR packages
find_package(LLVM REQUIRED CONFIG)
find_package(MLIR REQUIRED CONFIG)

message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(TableGen)
include(AddLLVM)
include(AddMLIR)

# 2. Include directories
include_directories(${LLVM_INCLUDE_DIRS} ${MLIR_INCLUDE_DIRS} )
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/include)
link_directories(${LLVM_LIBRARY_DIRS})

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/Stream")

set(LLVM_TARGET_DEFINITIONS include/StreamTablegen/StreamOps.td)
mlir_tablegen(include/Stream/StreamDialect.h.inc -gen-dialect-decls)
mlir_tablegen(include/Stream/StreamDialect.cpp.inc -gen-dialect-defs)
mlir_tablegen(include/Stream/StreamType.h.inc -gen-typedef-decls)
mlir_tablegen(include/Stream/StreamType.cpp.inc -gen-typedef-defs)
mlir_tablegen(include/Stream/StreamOps.h.inc -gen-op-decls)
mlir_tablegen(include/Stream/StreamOps.cpp.inc -gen-op-defs)
add_public_tablegen_target(MLIRStreamIncGen)

# include folder 
include_directories(include)

# NOTE: Moving jit libs to here. maybe only core is needed?
llvm_map_components_to_libnames(GEN_LIBS core)
llvm_map_components_to_libnames(JIT_LIBS core orcjit native)

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
  lib/IR/Operators.cpp

  # Stream
  lib/Stream/StreamDialect.cpp
  lib/Stream/StreamType.cpp

  # LLVMGen
  lib/LLVMGen/Gen.cpp
  lib/LLVMGen/Util.cpp
  lib/LLVMGen/FnGen.cpp
  lib/LLVMGen/Opt.cpp
)

add_dependencies(lib MLIRStreamIncGen)

# link library to executable
add_executable(app app/main.cpp)
add_executable(compiler app/compiler.cpp)

# Add this right after you define your executable or library target
target_compile_options(lib PRIVATE "-fno-rtti")
target_compile_options(compiler PRIVATE "-fno-rtti")

target_link_libraries(lib 
    PRIVATE
    MLIRIR
    MLIRFuncDialect
    MLIRArithDialect
    MLIRParser
    MLIRSupport
    LLVMSupport
    ${GEN_LIBS}
)
target_link_libraries(app lib)
target_link_libraries(compiler lib)

# NOTE: Added all of this as part of a test, have to move them back into the main project


add_library(jit 
  lib/JIT/jit.cpp
)
target_link_libraries(jit
  PRIVATE
  ${JIT_LIBS}
)

add_executable(jitapp app/jit.cpp)
target_link_libraries(jitapp jit lib)

