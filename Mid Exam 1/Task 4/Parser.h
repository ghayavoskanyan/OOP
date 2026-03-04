#pragma once
#include "Token.h"
#include "ASTNode.h"
#include "SymbolTable.h"
#include <memory>
#include <stack>
#include <vector>

class Lexer;

enum class ParserState {
    Start,
    Operand,
    Operator,
    Assignment,
    Error
};

class Parser {
private:
    Lexer& lexer;
    SymbolTable& symbolTable;
    ParserState currentState;
    std::stack<std::unique_ptr<ASTNode>> nodeStack;
    std::stack<char> operatorStack;
    Token currentToken;
    
    static const int STATE_COUNT = 4;
    static const int TOKEN_TYPE_COUNT = 7;
    
    ParserState transitionMatrix[STATE_COUNT][TOKEN_TYPE_COUNT];
    void initializeTransitionMatrix();
    int getTokenTypeIndex(TokenType type);
    void processToken();
    int getOperatorPrecedence(char op);
    void applyOperator();
    
public:
    Parser(Lexer& lex, SymbolTable& symTable);
    std::unique_ptr<ASTNode> parse();
    void reset();
};
