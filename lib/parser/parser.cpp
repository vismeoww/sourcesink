#include "parser/parser.hpp"
#include "parser/ast.hpp"

// TODO: remove later
#include <iostream>

#include <cassert>
#include <vector>
#include <numeric>

Parser::Parser(std::vector<Token> _tokens) { tokens = std::move(_tokens); pos_ = 0; tokens.push_back(Token{T_EOF, ""}); }

Token Parser::curr() {
  return tokens[pos_];
}

void Parser::consume(uint32_t n) {
  pos_ += n;
}

Result<statement::StmtPtr, std::string> Parser::parse() { 
  // Result<expr::ExprPtr, std::string> result = parseExpr();
  Result<statement::StmtPtr, std::string> result = parseStmt();
  return result;
}

Result<fn::FunctionPtr, std::string> Parser::parsefn() {
  Result<fn::FunctionPtr, std::string> result = parseFunction();
  return result;
}

Result<expr::ExprPtr, std::string> Parser::parseExpr() {
  auto result = parseTerm();
  if (result.isErr()) {
    return result;
  }
  expr::ExprPtr expr = result.getValue();
  while (curr().type == T_PLUS || curr().type == T_MINUS) {
    Token op = curr();
    consume();
    auto rhs = parseTerm();
    if (rhs.isErr()) {
      return rhs;
    }
    expr::BinOpType op_type = op.type == T_PLUS ? expr::kBinOpAdd : expr::kBinOpSub;
    expr = std::make_unique<expr::BinOp>(op_type, std::move(expr), std::move(rhs.getValue()));
  }
  return Result<expr::ExprPtr, std::string>(std::move(expr));
}

Result<expr::ExprPtr, std::string> Parser::parseTerm() {
  auto result = parseFactor();
  if (result.isErr()) {
    return result;
  }
  expr::ExprPtr expr = result.getValue();
  while (curr().type == T_MUL || curr().type == T_DIV) {
    Token op = curr();
    consume();
    auto rhs = parseFactor();
    if (rhs.isErr()) {
      return rhs;
    }
    expr::BinOpType op_type = op.type == T_MUL ? expr::kBinOpMul : expr::kBinOpDiv;
    expr = std::make_unique<expr::BinOp>(op_type, std::move(expr), std::move(rhs.getValue()));
  }
  return Result<expr::ExprPtr, std::string>(std::move(expr));
}

Result<expr::ExprPtr, std::string> Parser::parseFactor() {
  if (curr().type == T_OPEN_PAREN) {
    consume();
    auto result = parseExpr();
    if (result.isErr()) {
      return result;
    }
    if (curr().type != T_CLOSE_PAREN) {
      return Result<expr::ExprPtr, std::string>("expected ')'");
    }
    consume();
    return Result<expr::ExprPtr, std::string>(std::move(result.getValue()));
  }
  if (curr().type == T_NUMBER) {
    Result<expr::ExprPtr, std::string> result = Result<expr::ExprPtr, std::string>(std::make_unique<expr::IntValue>(std::stoi(curr().value)));
    consume();
    return std::move(result);
  }
  if (curr().type == T_IDENT) {
    save(); 
    auto result = parseFnCall();
    if (result.isErr()) {
      restore();
    } else {
      return std::move(result);
    }
    Result<expr::ExprPtr, std::string> result2 = Result<expr::ExprPtr, std::string>(std::make_unique<expr::Ident>(curr().value));
    consume();
    return std::move(result2);
  }
  return Result<expr::ExprPtr, std::string>("expected factor");
}

Result<expr::ExprPtr, std::string> Parser::parseFnCall() {
  if (curr().type != T_IDENT) {
    return Result<expr::ExprPtr, std::string>("expected identifier");
  }
  std::string name = curr().value;
  consume();
  if (curr().type != T_OPEN_PAREN) {
    return Result<expr::ExprPtr, std::string>("expected '('");
  }
  consume();
  std::vector<expr::ExprPtr> args;
  while (curr().type != T_CLOSE_PAREN) {
    auto result = parseExpr();
    if (result.isErr()) {
      return result;
    }
    args.push_back(std::move(result.getValue()));
    if (curr().type == T_CLOSE_PAREN) {
      break;
    }
    if (curr().type != T_COMMA) {
      return Result<expr::ExprPtr, std::string>("expected ','");
    }
    consume();
  }
  consume();
  return Result<expr::ExprPtr, std::string>(std::make_unique<expr::FunctionCall>(name, std::move(args)));
}

void Parser::save() {
  saved_pos_.push_back(pos_);
}

uint32_t Parser::restore() {
  assert(!saved_pos_.empty()&&"cannot restore from empty saved positions");
  uint32_t pos = saved_pos_.back();
  saved_pos_.pop_back();
  pos_ = pos;
  return pos;
}

Result<statement::StmtPtr, std::string> Parser::parseStmt() {
  save();
  auto result = parseReturnStmt();
  std::vector<std::string> errs;
  if (result.isErr()) {
    errs.push_back(result.getError());
    restore();
    result = parseAssignStmt();
  } else {
    return Result<statement::StmtPtr, std::string>(std::move(result.getValue()));
  }
  if (result.isErr()) {
    errs.push_back(result.getError());
    auto err = std::accumulate(errs.begin(), errs.end(), std::string(""));
    return Result<statement::StmtPtr, std::string>(err);
  }
  return Result<statement::StmtPtr, std::string>(std::move(result.getValue()));
}

Result<statement::StmtPtr, std::string> Parser::parseReturnStmt() {
  if (curr().type != T_RETURN) {
    return Result<statement::StmtPtr, std::string>("expected return");
  }
  consume();
  auto result = parseExpr();
  if (result.isErr()) {
    auto err = result.getError();
    return Result<statement::StmtPtr, std::string>(err);
  }
  if(curr().type != T_SEMICOL) {
    return Result<statement::StmtPtr, std::string>("expected EOL");
  }
  consume();
  return Result<statement::StmtPtr, std::string>(std::make_unique<statement::ReturnStmt>(std::move(result.getValue())));
}

Result<statement::StmtPtr, std::string> Parser::parseAssignStmt() {
  if (curr().type != T_IDENT) {
    return Result<statement::StmtPtr, std::string>("expected identifier");
  }
  std::string lhs = curr().value;
  consume();
  if (curr().type != T_EQUAL) {
    return Result<statement::StmtPtr, std::string>("expected =");
  }
  consume();
  auto result = parseExpr();
  if (result.isErr()) {
    auto err = result.getError();
    return Result<statement::StmtPtr, std::string>(err);
  }
  if(curr().type != T_SEMICOL) {
    return Result<statement::StmtPtr, std::string>("expected EOL");
  }
  consume();
  return Result<statement::StmtPtr, std::string>(std::make_unique<statement::AssignStmt>(lhs, std::move(result.getValue())));
}

Result<fn::FunctionPtr, std::string> Parser::parseFunction() {
  if (curr().type != T_FUNCTION) {
    return Result<fn::FunctionPtr, std::string>("expected fn");
  }
  consume();
  if (curr().type != T_IDENT) {
    return Result<fn::FunctionPtr, std::string>("expected function name");
  }
  std::string name = curr().value;
  consume();
  if (curr().type != T_OPEN_PAREN) {
    return Result<fn::FunctionPtr, std::string>("expected '('");
  }
  consume();
  std::vector<std::pair<std::string,std::string>> args;
  while (curr().type != T_CLOSE_PAREN) {
    if (curr().type != T_IDENT) {
      return Result<fn::FunctionPtr, std::string>("expected identifier");
    }
    std::string arg_name = curr().value;
    consume();
    if (curr().type != T_COLON) {
      return Result<fn::FunctionPtr, std::string>("expected ':'");
    }
    consume();
    if (curr().type != T_IDENT) {
      return Result<fn::FunctionPtr, std::string>("expected type");
    }
    std::string arg_type = curr().value;
    consume();
    args.push_back(std::make_pair(arg_name, arg_type));
    if (curr().type == T_CLOSE_PAREN) {
      break;
    }
    if (curr().type != T_COMMA) {
      return Result<fn::FunctionPtr, std::string>("expected ','");
    } else {
      consume();
    }
  }
  consume();
  std::vector<statement::StmtPtr> body;
  if (curr().type != T_OPEN_BRACE) {
    return Result<fn::FunctionPtr, std::string>("expected '{'");
  }
  consume();
  while (curr().type != T_CLOSE_BRACE) {
    auto result = parseStmt();
    if (result.isErr()) {
      Result<fn::FunctionPtr, std::string> err = Result<fn::FunctionPtr, std::string>(result.getError());
      return std::move(err);
    }
    body.push_back(std::move(result.getValue()));
  }
  consume();
  return Result<fn::FunctionPtr, std::string>(std::make_unique<fn::Function>(name, std::move(args), std::move(body)));
}
