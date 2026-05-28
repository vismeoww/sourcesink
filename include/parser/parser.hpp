#pragma once 
#include "lexer.hpp"
#include "ast.hpp"

#include <cassert>

template <typename T, typename E>
class Result {
  public:
    // Result(T value, E error) : value(value), error(error) {}
    Result(T value) : value(std::move(value)), error(E()) {}
    Result(E error) : value(T()), error(std::move(error)) {}
    bool isOk() { return error == E(); }
    bool isErr() { return error != E(); }
    T getValue() { assert(isOk()); return std::move(value); }
    E getError() { assert(isErr()); return std::move(error); }
    T& getValueRef() { assert(isOk()); return value; }
    E& getErrorRef() { assert(isErr()); return error; }

  private:
    T value;
    E error;
};

class Parser {
  public:
    Parser(std::vector<Token> tokens);
    Result<statement::StmtPtr, std::string> parse();
    Result<fn::FunctionPtr, std::string> parsefn();
    Result<module::ModulePtr, std::string> parsemod();

  private:
    uint32_t pos_;
    std::vector<uint32_t> saved_pos_;
    std::vector<Token> tokens;
    // consuming tokens 
    Token curr();
    void  consume(uint32_t n=1);
    bool isEOF(); 
    // functions for parsing
    Result<expr::ExprPtr, std::string> parseExpr();
    Result<expr::ExprPtr, std::string> parseTerm();
    Result<expr::ExprPtr, std::string> parseFactor();
    Result<expr::ExprPtr, std::string> parseInt();
    Result<expr::ExprPtr, std::string> parseFloat();
    Result<expr::ExprPtr, std::string> parseIdent();
    Result<expr::ExprPtr, std::string> parseGroup();
    Result<expr::ExprPtr, std::string> parseFnCall();
    // functions for parsing statements
    Result<statement::StmtPtr, std::string> parseStmt();
    Result<statement::StmtPtr, std::string> parseAssignStmt();
    Result<statement::StmtPtr, std::string> parseReturnStmt();
    // functions for parsing functions
    Result<fn::FunctionPtr, std::string> parseFunction();
    // parse module 
    Result<module::ModulePtr, std::string> parseModule();
    // functions for backtracking 
    void save();
    uint32_t restore();
};
