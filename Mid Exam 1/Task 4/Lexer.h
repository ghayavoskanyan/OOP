#pragma once
#include "Token.h"
#include <string>
#include <istream>

enum class LexerState {
    Start,
    InNumber,
    InName,
    InString,
    InOperator,
    InComment,
    InEscape
};

class Lexer {
private:
    std::istream& input;
    LexerState currentState;
    std::string currentToken;
    int line;
    int column;
    int currentChar;
    bool isEscape;
    
    //State matrix
    static const int STATE_COUNT = 7;
    static const int CHAR_TYPE_COUNT = 8;
    
    enum CharType {
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
    void handleState(char c);
    Token createToken(TokenType type);
    
public:
    Lexer(std::istream& is);
    Token getNextToken();
    void reset();
};