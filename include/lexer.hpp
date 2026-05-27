#pragma once

#include <string>
#include <vector>
#include "token.hpp"

#include <map>

const std::map<std::string, TokenType> keywords = {
    {"return", T_RETURN},
    {"fn", T_FUNCTION},
};


enum LexerState {
  READING_IDENT,
  DEFAULT,
};

class Lexer {
public:
    Lexer(std::string input);
    std::vector<Token> tokenize();

private:
    std::string input;
    std::vector<Token> tokens;
    int i = 0;
    LexerState state = DEFAULT;
    std::string ident;
    // helper functions 
    void saveIdent();
};


