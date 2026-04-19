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

    transitionMatrix[0][1] = ParserState::Operand;
    transitionMatrix[0][2] = ParserState::Operand;
    transitionMatrix[0][3] = ParserState::Operator;
    transitionMatrix[0][4] = ParserState::Operand;
    transitionMatrix[0][7] = ParserState::Operator;

    transitionMatrix[1][1] = ParserState::Operand;
    transitionMatrix[1][2] = ParserState::Operand;
    transitionMatrix[1][3] = ParserState::Operator;
    transitionMatrix[1][5] = ParserState::Operand;
    transitionMatrix[1][6] = ParserState::Assignment;
    transitionMatrix[1][7] = ParserState::Operator;
    transitionMatrix[1][0] = ParserState::Start;
    transitionMatrix[1][8] = ParserState::Operand;

    transitionMatrix[2][1] = ParserState::Operand;
    transitionMatrix[2][2] = ParserState::Operand;
    transitionMatrix[2][3] = ParserState::Operator;
    transitionMatrix[2][4] = ParserState::Operand;
    transitionMatrix[2][7] = ParserState::Operator;

    transitionMatrix[3][1] = ParserState::Operand;
    transitionMatrix[3][2] = ParserState::Operand;
    transitionMatrix[3][3] = ParserState::Operator;
    transitionMatrix[3][4] = ParserState::Operand;
    transitionMatrix[3][7] = ParserState::Operator;
}

int Parser::getTokenTypeIndex(TokenType type) {
    switch (type) {
        case TokenType::EndOfExpr:  return 0;
        case TokenType::Number:     return 1;
        case TokenType::Name:       return 2;
        case TokenType::Operator:   return 3;
        case TokenType::OpenParen:  return 4;
        case TokenType::CloseParen: return 5;
        case TokenType::Assignment: return 6;
        case TokenType::Comparison: return 7;
        case TokenType::Semicolon:  return 8;
        default: return 0;
    }
}

static char comparisonOpChar(const std::string& s) {
    if (s == ">")  return '>';
    if (s == "<")  return '<';
    if (s == ">=") return 'G';
    if (s == "<=") return 'L';
    if (s == "==") return 'E';
    if (s == "!=") return 'N';
    return '>';
}

void Parser::processToken() {
    switch (currentToken.type) {
        case TokenType::Number:
            nodeStack.push(std::make_unique<NumberNode>(std::stod(currentToken.value)));
            break;
        case TokenType::Name:
            nodeStack.push(std::make_unique<VariableNode>(currentToken.value, symbolTable));
            break;
        case TokenType::Comparison: {
            char op = comparisonOpChar(currentToken.value);
            while (!operatorStack.empty() && operatorStack.top() != '(' &&
                   getOperatorPrecedence(operatorStack.top()) >= getOperatorPrecedence(op))
                applyOperator();
            operatorStack.push(op);
            break;
        }
        case TokenType::Operator: {
            char op = currentToken.value[0];
            bool isUnary = (currentState == ParserState::Start ||
                            currentState == ParserState::Operator ||
                            currentState == ParserState::Assignment);
            if (isUnary && (op == '-' || op == '+')) {
                operatorStack.push(op == '-' ? 'u' : 'p');
            } else {
                while (!operatorStack.empty() && operatorStack.top() != '(' &&
                       getOperatorPrecedence(operatorStack.top()) >= getOperatorPrecedence(op))
                    applyOperator();
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
    if (op == 'u' || op == 'p') return 5;
    if (op == '*' || op == '/') return 4;
    if (op == '+' || op == '-') return 3;
    if (op == '>' || op == '<' || op == 'G' || op == 'L') return 2;
    if (op == 'E' || op == 'N') return 2;
    if (op == '=') return 1;
    return -1;
}

void Parser::applyOperator() {
    if (operatorStack.empty()) return;
    char op = operatorStack.top(); operatorStack.pop();

    if (op == 'u' || op == 'p') {
        if (nodeStack.empty()) throw std::runtime_error("No operand for unary operator");
        auto operand = std::move(nodeStack.top()); nodeStack.pop();
        nodeStack.push(std::make_unique<UnaryOpNode>(op == 'u' ? '-' : '+', std::move(operand)));
    } else {
        if (nodeStack.size() < 2) throw std::runtime_error("Not enough operands");
        auto right = std::move(nodeStack.top()); nodeStack.pop();
        auto left  = std::move(nodeStack.top()); nodeStack.pop();
        if (op == '=') {
            if (left->type != NodeType::VariableNode)
                throw std::runtime_error("Left side of assignment must be variable");
            std::string varName = static_cast<VariableNode*>(left.get())->name;
            nodeStack.push(std::make_unique<AssignmentNode>(varName, std::move(right), symbolTable));
        } else {
            nodeStack.push(std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right)));
        }
    }
}

std::unique_ptr<ASTNode> Parser::parse(bool stopAtCloseParen) {
    reset();
    while (true) {
        auto token = lexer.getNextToken();
        if (token.type == TokenType::EndOfExpr) break;
        if (token.type == TokenType::Semicolon) {
            lexer.pushBack(token);   // put back so caller can consume
            break;
        }
        if (token.type == TokenType::Keyword || token.type == TokenType::OpenBrace ||
            token.type == TokenType::CloseBrace) {
            lexer.pushBack(token);
            break;
        }
        if (token.type == TokenType::CloseParen) {
            if (stopAtCloseParen) {
                lexer.pushBack(token);
                break;
            }
            // normal handling inside parentheses
            int tidx = getTokenTypeIndex(token.type);
            if (currentState == ParserState::Start) break;
            currentToken = token;
            ParserState next = transitionMatrix[(int)currentState][tidx];
            if (next == ParserState::Error) break;
            processToken();
            currentState = next;
            continue;
        }

        currentToken = token;
        int tidx = getTokenTypeIndex(currentToken.type);
        ParserState next = transitionMatrix[(int)currentState][tidx];
        if (next == ParserState::Error) throw std::runtime_error("Syntax error at: " + currentToken.value);
        processToken();
        currentState = next;
    }
    while (!operatorStack.empty()) applyOperator();
    if (nodeStack.empty()) return nullptr;
    auto result = std::move(nodeStack.top()); nodeStack.pop();
    return result;
}

void Parser::reset() {
    currentState = ParserState::Start;
    while (!nodeStack.empty()) nodeStack.pop();
    while (!operatorStack.empty()) operatorStack.pop();
}