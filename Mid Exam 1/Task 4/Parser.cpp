#include "Parser.h"
#include "Lexer.h"
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <cctype>

enum class ParserStateIndex {
    Start = 0,
    AfterOperand,
    AfterOperator,
    Assignment,
    Error
};

int stateToIndex(ParserState state) {
    switch (state) {
        case ParserState::Start: return 0;
        case ParserState::Operand: return 1;
        case ParserState::Operator: return 2;
        case ParserState::Assignment: return 3;
        default: return 4;
    }
}

Parser::Parser(Lexer& lex, SymbolTable& symTable) : lexer(lex), symbolTable(symTable), currentState(ParserState::Start) {
    initializeTransitionMatrix();
}

void Parser::initializeTransitionMatrix() {
    for (int i = 0; i < STATE_COUNT; i++) {
        for (int j = 0; j < TOKEN_TYPE_COUNT; j++) {
            transitionMatrix[i][j] = ParserState::Error;
        }
    }
    
    //index 0
    transitionMatrix[0][getTokenTypeIndex(TokenType::Number)] = ParserState::Operand;
    transitionMatrix[0][getTokenTypeIndex(TokenType::Name)] = ParserState::Operand;
    transitionMatrix[0][getTokenTypeIndex(TokenType::OpenParen)] = ParserState::Operator;
    transitionMatrix[0][getTokenTypeIndex(TokenType::Operator)] = ParserState::Operator;
    
    //index 1
    transitionMatrix[1][getTokenTypeIndex(TokenType::Operator)] = ParserState::Operator;
    transitionMatrix[1][getTokenTypeIndex(TokenType::CloseParen)] = ParserState::Operator;
    transitionMatrix[1][getTokenTypeIndex(TokenType::EndOfExpr)] = ParserState::Start;
    transitionMatrix[1][getTokenTypeIndex(TokenType::Assignment)] = ParserState::Assignment;
    
    //index 2
    transitionMatrix[2][getTokenTypeIndex(TokenType::Number)] = ParserState::Operand;
    transitionMatrix[2][getTokenTypeIndex(TokenType::Name)] = ParserState::Operand;
    transitionMatrix[2][getTokenTypeIndex(TokenType::OpenParen)] = ParserState::Operator;
    transitionMatrix[2][getTokenTypeIndex(TokenType::Operator)] = ParserState::Operator;
    
    //index 3
    transitionMatrix[3][getTokenTypeIndex(TokenType::Number)] = ParserState::Operand;
    transitionMatrix[3][getTokenTypeIndex(TokenType::Name)] = ParserState::Operand;
    transitionMatrix[3][getTokenTypeIndex(TokenType::OpenParen)] = ParserState::Operator;
}

int Parser::getTokenTypeIndex(TokenType type) {
    switch (type) {
        case TokenType::EndOfExpr: return 0;
        case TokenType::Number: return 1;
        case TokenType::Name: return 2;
        case TokenType::Operator: return 3;
        case TokenType::OpenParen: return 4;
        case TokenType::CloseParen: return 5;
        case TokenType::Assignment: return 6;
        default: return 0;
    }
}

int Parser::getOperatorPrecedence(char op) {
    switch (op) {
        case '=': return 0;
        case '+':
        case '-': return 1;
        case '*':
        case '/': return 2;
        default: return 0;
    }
}

void Parser::applyOperator() {
    if (operatorStack.empty()) return;
    
    char op = operatorStack.top();
    operatorStack.pop();
    
    if (op == '=') {
        if (nodeStack.size() < 2) {
            throw std::runtime_error("Invalid assignment: missing expression");
        }
        auto expr = std::move(nodeStack.top());
        nodeStack.pop();
        if (nodeStack.empty() || nodeStack.top()->type != NodeType::VariableNode) {
            throw std::runtime_error("Invalid assignment: left side must be a variable");
        }
        auto varNode = static_cast<VariableNode*>(nodeStack.top().get());
        std::string varName = varNode->name;
        nodeStack.pop();
        nodeStack.push(std::make_unique<AssignmentNode>(varName, std::move(expr)));
        return;
    }
    
    if (nodeStack.empty()) return;
    
    if (nodeStack.size() == 1) {
        if (op == '+' || op == '-') {
            auto right = std::move(nodeStack.top());
            nodeStack.pop();
            nodeStack.push(std::make_unique<UnaryOpNode>(op, std::move(right)));
        }
        return;
    }
    
    auto right = std::move(nodeStack.top());
    nodeStack.pop();
    auto left = std::move(nodeStack.top());
    nodeStack.pop();
    nodeStack.push(std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right)));
}

void Parser::processToken() {
    switch (currentToken.type) {
        case TokenType::Number: {
            double value = std::stod(currentToken.value);
            nodeStack.push(std::make_unique<NumberNode>(value));
            break;
        }
        
        case TokenType::Name: {
            if (!symbolTable.exists(currentToken.value)) {
                symbolTable.addSymbol(currentToken.value, 0.0);
            }
            nodeStack.push(std::make_unique<VariableNode>(currentToken.value));
            break;
        }
        
        case TokenType::Operator: {
            char op = currentToken.value[0];
            
            bool isUnary = (nodeStack.empty() || currentState == ParserState::Operator) && (op == '+' || op == '-');
            if (isUnary) {
                operatorStack.push(op);
                break;
            }
            
            while (!operatorStack.empty() && operatorStack.top() != '(' &&
                   getOperatorPrecedence(operatorStack.top()) >= getOperatorPrecedence(op)) {
                applyOperator();
            }
            operatorStack.push(op);
            break;
        }
        
        case TokenType::OpenParen: {
            operatorStack.push('(');
            break;
        }
        
        case TokenType::CloseParen: {
            while (!operatorStack.empty() && operatorStack.top() != '(') {
                applyOperator();
            }
            if (!operatorStack.empty() && operatorStack.top() == '(') {
                operatorStack.pop();
            } else {
                throw std::runtime_error("Mismatched parentheses");
            }
            break;
        }
        
        case TokenType::Assignment: {
            operatorStack.push('=');
            break;
        }
        
        default:
            break;
    }
}

std::unique_ptr<ASTNode> Parser::parse() {
    currentState = ParserState::Start;
    
    while (!nodeStack.empty()) nodeStack.pop();
    while (!operatorStack.empty()) operatorStack.pop();
    
    while (true) {
        currentToken = lexer.getNextToken();
        
        if (currentToken.type == TokenType::EndOfExpr) {
            break;
        }
        
        int tokenIndex = getTokenTypeIndex(currentToken.type);
        ParserState newState = transitionMatrix[static_cast<int>(currentState)][tokenIndex];
        
        if (newState == ParserState::Error) {
            throw std::runtime_error("Syntax error at line " + std::to_string(currentToken.line) + 
                                    ", column " + std::to_string(currentToken.column) + 
                                    ", unexpected token: " + currentToken.value);
        }
        
        processToken();
        currentState = newState;
    }
    
    while (!operatorStack.empty()) {
        applyOperator();
    }
    
    if (nodeStack.size() == 1) {
        return std::move(nodeStack.top());
    } else if (nodeStack.empty()) {
        return nullptr;
    } else {
        throw std::runtime_error("Invalid expression: too many operands");
    }
}

void Parser::reset() {
    currentState = ParserState::Start;
    while (!nodeStack.empty()) nodeStack.pop();
    while (!operatorStack.empty()) operatorStack.pop();
}