#pragma once
#include "Token.h"
#include <string>
#include <istream>

enum class LexerState {
    Start,
    InNumber,
    InName,
    InString,
    InOperator
};

class Lexer {
private:
    std::istream& input;
    LexerState currentState;
    std::string currentToken;
    int line;
    int column;
    bool isEscape;
   
    static const int STATE_COUNT = 5;
    static const int CHAR_TYPE_COUNT = 8; 
   
    enum class CharType {
        Digit,
        Letter,
        Whitespace,
        Operator_,
        Paren,
        Quote,
        Newline_,
        Other
    };
   
    LexerState transitionMatrix[STATE_COUNT][CHAR_TYPE_COUNT];
    void initializeTransitionMatrix();
    CharType getCharType(char c);
   
public:
    Lexer(std::istream& is);
    Token getNextToken();
};