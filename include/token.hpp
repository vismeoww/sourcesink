#pragma once

#include <string>

enum TokenType {
    T_IDENT,
    T_NUMBER,
    T_SEMICOL,
    T_ERROR,
    T_OPEN_PAREN,
    T_CLOSE_PAREN,
    T_OPEN_BRACE,
    T_CLOSE_BRACE,
    T_OPEN_BRACKET,
    T_CLOSE_BRACKET,
    T_COMMA,
    T_EQUAL,
    T_COLON,
    //operators 
    T_PLUS,
    T_MINUS,
    T_MUL,
    T_DIV,
    T_MOD,
    T_EQ,
    T_NEQ,
    T_LT,
    T_GT,
    T_LTE,
    T_GTE,
    T_AND,
    T_OR,
    T_NOT,
    // keywords
    T_RETURN,
    T_FUNCTION,
    // EOF 
    T_EOF
};

struct Token {
    TokenType type;
    std::string value;
};

std::string tokenTypeToString(TokenType type);

std::string tokenToString(Token token);
