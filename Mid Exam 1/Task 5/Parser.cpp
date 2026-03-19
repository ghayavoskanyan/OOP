#include "Parser.h"
#include "Lexer.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(Lexer& lex, SymbolTable& symTable) 
    : lexer(lex), symbolTable(symTable), currentState(ParserState::Start) {
    initializeTransitionMatrix();
}

void Parser::initializeTransitionMatrix() {
    for (int i = 0; i < STATE_COUNT; i++) 
        for (int j = 0; j < TOKEN_TYPE_COUNT; j++) 
            transitionMatrix[i][j] = ParserState::Error;

    // Start state
    transitionMatrix[0][1] = ParserState::Operand;    // Number
    transitionMatrix[0][2] = ParserState::Operand;    // Name
    transitionMatrix[0][3] = ParserState::Operator;   // Operator (for unary -/+)
    transitionMatrix[0][4] = ParserState::Operand;    // OpenParen '('

    // Operand state
    transitionMatrix[1][1] = ParserState::Operand;    // Number stays in Operand
    transitionMatrix[1][2] = ParserState::Operand;    // Name stays in Operand
    transitionMatrix[1][3] = ParserState::Operator;   // Operator (binary)
    transitionMatrix[1][5] = ParserState::Operand;    // CloseParen ')'
    transitionMatrix[1][6] = ParserState::Assignment; // Assignment '='
    transitionMatrix[1][0] = ParserState::Start;      // End of expression

    // Operator state
    transitionMatrix[2][1] = ParserState::Operand;    // Number
    transitionMatrix[2][2] = ParserState::Operand;    // Variable
    transitionMatrix[2][3] = ParserState::Operator;   // Another operator (unary)
    transitionMatrix[2][4] = ParserState::Operand;    // '('

    // Assignment state
    transitionMatrix[3][1] = ParserState::Operand;    // Number
    transitionMatrix[3][2] = ParserState::Operand;    // Variable
    transitionMatrix[3][3] = ParserState::Operator;   // Operator (unary)
    transitionMatrix[3][4] = ParserState::Operand;    // '('
}

int Parser::getTokenTypeIndex(TokenType type) {
    switch (type) {
        case TokenType::EndOfExpr: return 0;
        case TokenType::Number:    return 1;
        case TokenType::Name:      return 2;
        case TokenType::Operator:  return 3;
        case TokenType::OpenParen: return 4;
        case TokenType::CloseParen: return 5;
        case TokenType::Assignment: return 6;
        default: return 0;
    }
}

void Parser::processToken() {
    switch (currentToken.type) {
        case TokenType::Number:
            nodeStack.push(std::make_unique<NumberNode>(std::stod(currentToken.value)));
            break;
            
        case TokenType::Name:
            nodeStack.push(std::make_unique<VariableNode>(currentToken.value, symbolTable));
            break;
            
        case TokenType::Operator: {
            char op = currentToken.value[0];
            // Check if it's a unary operator
            bool isUnary = (currentState == ParserState::Start ||
                           currentState == ParserState::Operator ||
                           currentState == ParserState::Assignment);
            
            if (isUnary && (op == '-' || op == '+')) {
                // For unary minus, push a special marker
                operatorStack.push(op == '-' ? 'u' : 'p');
            } else {
                // Binary operator - apply precedence
                while (!operatorStack.empty() && operatorStack.top() != '(' && 
                       getOperatorPrecedence(operatorStack.top()) >= getOperatorPrecedence(op)) {
                    applyOperator();
                }
                operatorStack.push(op);
            }
            break;
        }
        
        case TokenType::OpenParen: 
            operatorStack.push('('); 
            break;
            
        case TokenType::CloseParen:
            while (!operatorStack.empty() && operatorStack.top() != '(') {
                applyOperator();
            }
            if (!operatorStack.empty() && operatorStack.top() == '(') {
                operatorStack.pop();
            }
            break;
            
        case TokenType::Assignment: 
            // Assignment has lowest precedence
            while (!operatorStack.empty() && operatorStack.top() != '(') {
                applyOperator();
            }
            operatorStack.push('='); 
            break;
            
        default: break;
    }
}

int Parser::getOperatorPrecedence(char op) {
    if (op == 'u' || op == 'p') return 3;  // Unary operators
    if (op == '*' || op == '/') return 2;
    if (op == '+' || op == '-') return 1;
    if (op == '=') return 0;
    return -1;
}

void Parser::applyOperator() {
    if (operatorStack.empty()) return;
    
    char op = operatorStack.top(); 
    operatorStack.pop();

    if (op == 'u' || op == 'p') {
        // Unary operator
        if (nodeStack.empty()) {
            throw std::runtime_error("No operand for unary operator");
        }
        auto operand = std::move(nodeStack.top()); 
        nodeStack.pop();
        nodeStack.push(std::make_unique<UnaryOpNode>(op == 'u' ? '-' : '+', std::move(operand)));
    } else {
        // Binary operator
        if (nodeStack.size() < 2) {
            throw std::runtime_error("Not enough operands for binary operator");
        }
        
        auto right = std::move(nodeStack.top()); 
        nodeStack.pop();
        auto left = std::move(nodeStack.top()); 
        nodeStack.pop();

        if (op == '=') {
            // Check if left side is a variable
            if (left->type != NodeType::VariableNode) {
                throw std::runtime_error("Left side of assignment must be a variable");
            }
            
            std::string varName = static_cast<VariableNode*>(left.get())->name;
            nodeStack.push(std::make_unique<AssignmentNode>(varName, std::move(right), symbolTable));
        } else {
            nodeStack.push(std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right)));
        }
    }
}

std::unique_ptr<ASTNode> Parser::parse() {
    reset();
    
    while (true) {
        auto token = lexer.getNextToken();
        if (token.type == TokenType::EndOfExpr) break;
        
        currentToken = token;
        int tidx = getTokenTypeIndex(currentToken.type);
        
        if (tidx < 0 || tidx >= TOKEN_TYPE_COUNT || 
            (int)currentState < 0 || (int)currentState >= STATE_COUNT) {
            throw std::runtime_error("Invalid state or token type");
        }
        
        ParserState next = transitionMatrix[(int)currentState][tidx];

        if (next == ParserState::Error) {
            throw std::runtime_error("Syntax error at: " + currentToken.value);
        }
        
        processToken();
        currentState = next;
    }
    
    // Apply remaining operators
    while (!operatorStack.empty()) {
        applyOperator();
    }
    
    if (nodeStack.empty()) {
        return nullptr;
    }
    
    auto result = std::move(nodeStack.top());
    nodeStack.pop();
    
    if (!nodeStack.empty()) {
        throw std::runtime_error("Invalid expression: too many operands");
    }
    
    return result;
}

void Parser::reset() {
    currentState = ParserState::Start;
    while(!nodeStack.empty()) nodeStack.pop();
    while(!operatorStack.empty()) operatorStack.pop();
}