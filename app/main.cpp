#include <iostream>
#include <string>
#include "lexer.hpp"
#include "parser.hpp"

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

int main() {
    testParser("a = b + c;");
    testParser("return a + b * c;");
    testParser("return a(b, c(1));");
    testFnParser("fn add(a:int, b:int) { return a + b; }");
    testFnParser("fn calc(a:int,b:int){ c = 1 + exp(a); return sin(c*pi*b);}");
    return 0;
}
