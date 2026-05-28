
#include <iostream>
#include <fstream>
#include <string>
#include "parser/lexer.hpp"
#include "parser/parser.hpp"

#include "IR/MLIRGen.hpp"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"

int main(int argc, char** argv) {
  // read the input file 
  std::string inputFile = argv[1];
  std::ifstream input(inputFile);
  if (!input.is_open()) {
    std::cout << "error: cannot open file " << inputFile << std::endl;
    return -1;
  }
  std::string inputStr((std::istreambuf_iterator<char>(input)),
                       std::istreambuf_iterator<char>());
  input.close();
  // parse the input file 
  Lexer lexer(inputStr);
  auto tokens = lexer.tokenize();
  Parser parser(tokens);
  auto result = parser.parsemod();
  if (result.isErr()) {
    std::cout << "error: " << result.getError() << std::endl;
    return -1;
  }

  auto fn = result.getValue();
  mlir::MLIRContext context;
  context.loadDialect<mlir::func::FuncDialect, mlir::arith::ArithDialect>();
  MLIRGenImpl gen(context);
  auto module = gen.mlirgen(std::move(fn));
  std::cout << ";----- Generated MLIR -----" << std::endl;
  module.dump();
  return 0;
}
