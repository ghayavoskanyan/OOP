#pragma once
#include <string>

enum class TokenType {
    EndOfExpr,
    Number,
    Name,
    Operator,
    OpenParen,
    CloseParen,
    Assignment,
    Unknown,
    Error
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token(TokenType t = TokenType::Unknown, const std::string& v = "", int l = 0, int c = 0)
        : type(t), value(v), line(l), column(c) {}
};
