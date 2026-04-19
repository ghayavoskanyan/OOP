#pragma once
#include "Token.h"
#include <string>
#include <istream>
#include <unordered_map>

enum class LexerState { Start, InNumber, InDecimal, InName, InOperator };

class Lexer {
private:
    std::istream& input;
    LexerState currentState;
    std::string currentToken;
    int line;
    int column;
    std::unordered_map<std::string, TokenType> keywords;

    Token pendingToken;
    bool hasPendingToken;

    static const int STATE_COUNT = 5;
    static const int CHAR_TYPE_COUNT = 9;

    enum class CharType { Digit, Letter, Whitespace, Operator_, Paren, Brace, Semicolon, Dot, Other };

    LexerState transitionMatrix[STATE_COUNT][CHAR_TYPE_COUNT];
    void initializeTransitionMatrix();
    CharType getCharType(char c);
    void initializeKeywords();

public:
    Lexer(std::istream& is);
    Token getNextToken();
    void pushBack(const Token& token);
    void reset(); 
};