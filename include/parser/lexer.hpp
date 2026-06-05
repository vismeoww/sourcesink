#pragma once

#include <string>
#include <vector>
#include "token.hpp"

#include <map>
#include <stdint.h>

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
    void advance();

private:
    std::string input;
    std::vector<Token> tokens;
    int i = 0;
    uint32_t line = 0;
    uint32_t col = 0;
    LexerState state = DEFAULT;
    std::string ident;
    // helper functions
    void saveIdent();
};
