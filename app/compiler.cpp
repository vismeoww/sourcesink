
#include <iostream>
#include <fstream>
#include <string>
#include "parser/lexer.hpp"
#include "parser/parser.hpp"

#include "IR/MLIRGen.hpp"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"

#include "Stream/StreamDialect.h"

struct Options {
  bool help = false;
  std::optional<std::string> inputFile;
  bool debug = false;
  bool parse = false;
};

Options parseOptions(int argc, char** argv) {
  Options options;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      options.help = true;
    } else if (arg == "-d" || arg == "--debug") {
      options.debug = true;
    } else if (arg == "-p" || arg == "--parse") {
      options.parse = true;
    } else {
      options.inputFile = arg;
    }
  }
  return options;
}

void printHelp() {
    std::cout << "Usage: compiler [options] inputFile" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help      Print this help message" << std::endl;
    std::cout << "  -d, --debug     Enable debug mode" << std::endl;
    std::cout << "  -p, --parse     Parse the input file and print the AST" << std::endl;
}

int main(int argc, char** argv) {

  Options options = parseOptions(argc, argv);
  if (options.help) {
    printHelp();
    return 0;
  }

  if (!options.inputFile) {
    std::cout << "error: input file is required" << std::endl;
    printHelp();
    return -1;
  }

  // read the input file 
  std::string inputFile = options.inputFile.value();

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

  if (options.parse) {
    std::cout << ";----- TOKENS -----" << std::endl;
    for (auto& token : tokens) {
      std::cout << tokenToString(token) << std::endl;
    }
    std::cout << ";----- AST -----" << std::endl;
    auto res = result.getValue()->toString();
    std::cout << res << std::endl;
    return 0;
  }

  auto mod = result.getValue();
  mlir::MLIRContext context;
  context.loadDialect<mlir::func::FuncDialect, mlir::arith::ArithDialect, mlir::stream::StreamDialect>();
  std::string moduleName = inputFile.substr(inputFile.find_last_of("/") + 1);
  MLIRGenImpl gen(context, moduleName);
  auto module = gen.mlirgen(std::move(mod));

  mlir::OpPrintingFlags flags;

  if (options.debug) {
    flags.enableDebugInfo(true);
  }

  std::cout << ";----- Generated MLIR -----" << std::endl;
  module.print(llvm::outs(),flags);

  return 0;
}
