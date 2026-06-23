#include <iostream>
#include <string>
#include "parser/lexer.hpp"
#include "parser/parser.hpp"

#include "IR/MLIRGen.hpp"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"

void testParser(std::string input) {
    Lexer lexer(input);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto result = parser.parse();
    if (result.isErr()) {
        std::cout << "error: " << result.getError() << std::endl;
    } else {
        std::cout << "parsed successfully : " << result.getValue()->toString() << std::endl;
    }
}

void testFnParser(std::string input) {
    Lexer lexer(input);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto result = parser.parsefn();
    if (result.isErr()) {
        std::cout << "error: " << result.getError() << std::endl;
    } else {
        std::cout << "parsed successfully : " << result.getValue()->toString() << std::endl;
    }
}

void dumpMLIR(std::string input) {
    std::cout << "------- Processing Program --------" << std::endl;
    std::cout << input << std::endl;
    std::cout << "----------------------------------" << std::endl;
    Lexer lexer(input);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto result = parser.parsefn();
    if (result.isErr()) {
        std::cout << "error: " << result.getError() << std::endl;
    } else {
        auto fn = result.getValue();
        mlir::MLIRContext context;
        context.loadDialect<mlir::func::FuncDialect, mlir::arith::ArithDialect>();
        MLIRGenImpl gen(context, "test");
        auto module = gen.mlirgen(std::move(fn));
        std::cout << "----- Generated MLIR -----" << std::endl;
        module.dump();
        std::cout << "--------------------------" << std::endl;
    }
}

void testModuleParser(std::string input) {
    Lexer lexer(input);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto result = parser.parsemod();
    if (result.isErr()) {
        std::cout << "error: " << result.getError() << std::endl;
    } else {
        std::cout << "parsed successfully : " << result.getValue()->toString() << std::endl;
    }
}

int main() {
    testModuleParser("fn add(a:i32,b:i32){return a+b;} fn function(a:i64,b:i64){ c = a + b; d = c * 2; e = d + 1; return e; }");
    testModuleParser("fn add(a:i32,b:i32){return a+b;}");
    return 0;
}
