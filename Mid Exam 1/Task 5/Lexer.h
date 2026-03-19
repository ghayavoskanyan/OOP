#pragma once
#include "Token.h"
#include <string>
#include <istream>

enum class LexerState {
    Start,
    InNumber,
    InName,
    InOperator
};

class Lexer {
private:
    std::istream& input;
    LexerState currentState;
    std::string currentToken;
    int line;
    int column;
   
    static const int STATE_COUNT = 4;
    static const int CHAR_TYPE_COUNT = 7; 
   
    enum class CharType {
        Digit,
        Letter,
        Whitespace,
        Operator_,
        Paren,
        Other
    };
   
    LexerState transitionMatrix[STATE_COUNT][CHAR_TYPE_COUNT];
    void initializeTransitionMatrix();
    CharType getCharType(char c);
   
public:
    Lexer(std::istream& is);
    Token getNextToken();
};