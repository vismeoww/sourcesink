#include "lexer.hpp"

Lexer::Lexer(std::string input) { this->input = input; }

const bool isReservedChar(char c) {
  return c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == '[' ||
         c == ']' || c == ',' || c == '=' || c == ':' || c == '+' || c == '-' || 
         c == '*' || c == '/';
}
const bool isWhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

TokenType getTokenType(char c) {
  switch (c) {
  case ';':
    return T_SEMICOL;
  case '(':
    return T_OPEN_PAREN;
  case ')':
    return T_CLOSE_PAREN;
  case '{':
    return T_OPEN_BRACE;
  case '}':
    return T_CLOSE_BRACE;
  case '[':
    return T_OPEN_BRACKET;
  case ']':
    return T_CLOSE_BRACKET;
  case ',':
    return T_COMMA;
  case '=':
    return T_EQUAL;
  case ':':
    return T_COLON;
  case '+':
    return T_PLUS;
  case '-':
    return T_MINUS;
  case '*':
    return T_MUL;
  case '/':
    return T_DIV;
  default:
    return T_ERROR;
  }
}


void Lexer::saveIdent() {
  // if it's a keyword, save it as a keyword
  if(keywords.find(ident) != keywords.end()) {
    Token t = {keywords.at(ident), ident};
    tokens.push_back(t);
    ident = "";
    state = DEFAULT;
    return;
  }
  // if ident is an int, save it as a number
  if (ident.find_first_not_of("0123456789") == std::string::npos) {
    Token t = {T_NUMBER, ident};
    tokens.push_back(t);
    ident = "";
    state = DEFAULT;
    return;
  }
  // if not save it as an identifier
  Token t = {T_IDENT, ident};
  tokens.push_back(t);
  ident = "";
  state = DEFAULT;
}

std::vector<Token> Lexer::tokenize() {
  while (i < input.length()) {
    if (isWhitespace(input[i])) {
      if (state == READING_IDENT) {
        saveIdent();
      }
      i++;
      continue;
    }
    if (isReservedChar(input[i])) {
      if (state == READING_IDENT) {
        saveIdent();
      }
      TokenType tp = getTokenType(input[i]);
      Token t = {tp, input.substr(i, 1)};
      tokens.push_back(t);
      i++;
      continue;
    }
    if (input[i] >= 'a' && input[i] <= 'z' ||
        input[i] >= 'A' && input[i] <= 'Z' ||
        input[i] >= '0' && input[i] <= '9' ||
        input[i] == '_') {
      state = READING_IDENT;
      ident += input[i];
      i++;
      continue;
    }
    i++;
  }
  return tokens;
}
