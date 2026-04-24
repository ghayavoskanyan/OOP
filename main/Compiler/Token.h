#pragma once
#include <string>

enum class TokenType {
    EndOfExpr,
    Number,
    Name,
    Operator,
    Comparison,
    OpenParen,
    CloseParen,
    OpenBrace,
    CloseBrace,
    Semicolon,
    Colon,
    Comma,
    Dot,
    Assignment,
    Keyword,
    Unknown
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    Token(TokenType t = TokenType::Unknown, const std::string& v = "", int l = 0, int c = 0);
};