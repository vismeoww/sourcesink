#include "token.hpp"

std::string tokenTypeToString(TokenType type) {
  switch (type) {
  case T_IDENT:
    return "IDENT";
  case T_NUMBER:
    return "NUMBER";
  case T_SEMICOL:
    return "EOL";
  case T_ERROR:
    return "ERROR";
  case T_OPEN_PAREN:
    return "OPEN_PAREN";
  case T_CLOSE_PAREN:
    return "CLOSE_PAREN";
  case T_OPEN_BRACE:
    return "OPEN_BRACE";
  case T_CLOSE_BRACE:
    return "CLOSE_BRACE";
  case T_OPEN_BRACKET:
    return "OPEN_BRACKET";
  case T_CLOSE_BRACKET:
    return "CLOSE_BRACKET";
  case T_COMMA:
    return "COMMA";
  case T_EQUAL:
    return "EQUAL";
  case T_COLON:
    return "COLON";
  case T_PLUS:
    return "PLUS";
  case T_MINUS:
    return "MINUS";
  case T_MUL:
    return "MUL";
  case T_DIV:
    return "DIV";
  default:
    return "UNKNOWN";
  }
}

std::string tokenToString(Token token) {
  if (token.type == T_IDENT) {
    return tokenTypeToString(token.type) + ": " + token.value;
  } else if (token.type == T_NUMBER) {
    return tokenTypeToString(token.type) + ": " + token.value;
  } else {
    return tokenTypeToString(token.type);
  }
}
