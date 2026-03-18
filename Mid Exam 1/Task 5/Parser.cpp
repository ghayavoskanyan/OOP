#include "Parser.h"
#include "Lexer.h"
#include <stdexcept>

Parser::Parser(Lexer& lex, SymbolTable& symTable) 
    : lexer(lex), symbolTable(symTable), currentState(ParserState::Start) {
    initializeTransitionMatrix();
}

void Parser::initializeTransitionMatrix() {
    for (int i = 0; i < STATE_COUNT; i++) 
        for (int j = 0; j < TOKEN_TYPE_COUNT; j++) 
            transitionMatrix[i][j] = ParserState::Error;

    // --- Start state (Վիճակ 0) ---
    transitionMatrix[0][1] = ParserState::Operand;    // Number
    transitionMatrix[0][2] = ParserState::Operand;    // Name
    transitionMatrix[0][3] = ParserState::Operator;   // Այս տողը թույլ է տալիս '-' կամ '+' սկզբում
    transitionMatrix[0][4] = ParserState::Operator;   // OpenParen '('

    // --- Operand state (Վիճակ 1) ---
    transitionMatrix[1][3] = ParserState::Operator;   // Operator (+, -, *, /)
    transitionMatrix[1][5] = ParserState::Operator;   // CloseParen ')'
    transitionMatrix[1][6] = ParserState::Assignment; // Assignment '='
    transitionMatrix[1][0] = ParserState::Start;      // End of expression

    // --- Operator state (Վիճակ 2) ---
    transitionMatrix[2][1] = ParserState::Operand;    // Օպերատորից հետո թիվ
    transitionMatrix[2][2] = ParserState::Operand;    // Օպերատորից հետո փոփոխական
    transitionMatrix[2][3] = ParserState::Operator;   // Օպերատորից հետո այլ օպերատոր (օր.՝ - - 5)
    transitionMatrix[2][4] = ParserState::Operator;   // Օպերատորից հետո '('

    // --- Assignment state (Վիճակ 3) ---
    transitionMatrix[3][1] = ParserState::Operand;    // '=' -ից հետո թիվ
    transitionMatrix[3][2] = ParserState::Operand;    // '=' -ից հետո փոփոխական
    transitionMatrix[3][3] = ParserState::Operator;   // '=' -ից հետո '-' (օր.՝ w = -5)
    transitionMatrix[3][4] = ParserState::Operator;   // '=' -ից հետո '('
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
            nodeStack.push(std::make_unique<VariableNode>(currentToken.value));
            break;
        case TokenType::Operator: {
            char op = currentToken.value[0];
            bool isUnary = (
                currentState == ParserState::Start ||
                currentState == ParserState::Operator ||
                currentState == ParserState::Assignment
            );
            if (isUnary) {
                operatorStack.push(op == '-' ? 'u' : 'p');
            } else {
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
            while (!operatorStack.empty() && operatorStack.top() != '(') applyOperator();
            if (!operatorStack.empty()) operatorStack.pop();
            break;
        case TokenType::Assignment: 
            while (!operatorStack.empty() && operatorStack.top() != '(') applyOperator();
            operatorStack.push('='); 
            break;
        default: break;
    }
}

int Parser::getOperatorPrecedence(char op) {
    if (op == 'u' || op == 'p') return 3;
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
        if (nodeStack.empty()) return;
        auto operand = std::move(nodeStack.top()); nodeStack.pop();
        nodeStack.push(std::make_unique<UnaryOpNode>(op == 'u' ? '-' : '+', std::move(operand)));
    } else {
        if (nodeStack.size() < 2) return;
        auto right = std::move(nodeStack.top()); nodeStack.pop();
        auto left = std::move(nodeStack.top()); nodeStack.pop();

        if (op == '=') {
            if (left->type != NodeType::VariableNode) 
                throw std::runtime_error("L-value required for assignment");
            
            std::string varName = static_cast<VariableNode*>(left.get())->name;
            nodeStack.push(std::make_unique<AssignmentNode>(varName, std::move(right)));
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
        ParserState next = transitionMatrix[(int)currentState][tidx];

        if (next == ParserState::Error) throw std::runtime_error("Syntax error at: " + currentToken.value);
        
        processToken();
        currentState = next;
    }
    while (!operatorStack.empty()) applyOperator();
    return nodeStack.empty() ? nullptr : std::move(nodeStack.top());
}

void Parser::reset() {
    currentState = ParserState::Start;
    while(!nodeStack.empty()) nodeStack.pop();
    while(!operatorStack.empty()) operatorStack.pop();
}