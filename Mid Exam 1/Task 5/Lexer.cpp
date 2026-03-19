#include "Lexer.h"
#include <cctype>

Lexer::Lexer(std::istream& is) : input(is), currentState(LexerState::Start), line(1), column(0) {
    initializeTransitionMatrix();
}

void Lexer::initializeTransitionMatrix() {
    for (int i = 0; i < STATE_COUNT; i++) {
        for (int j = 0; j < CHAR_TYPE_COUNT; j++) {
            transitionMatrix[i][j] = LexerState::Start;
        }
    }

    transitionMatrix[(int)LexerState::Start][(int)CharType::Digit] = LexerState::InNumber;
    transitionMatrix[(int)LexerState::Start][(int)CharType::Letter] = LexerState::InName;
    transitionMatrix[(int)LexerState::Start][(int)CharType::Operator_] = LexerState::InOperator;
    
    transitionMatrix[(int)LexerState::InNumber][(int)CharType::Digit] = LexerState::InNumber;
    transitionMatrix[(int)LexerState::InNumber][(int)CharType::Operator_] = LexerState::InOperator;
    
    transitionMatrix[(int)LexerState::InName][(int)CharType::Letter] = LexerState::InName;
    transitionMatrix[(int)LexerState::InName][(int)CharType::Digit] = LexerState::InName;
    transitionMatrix[(int)LexerState::InName][(int)CharType::Operator_] = LexerState::InOperator;
}

Lexer::CharType Lexer::getCharType(char c) {
    if (isdigit(c)) return CharType::Digit;
    if (isalpha(c)) return CharType::Letter;
    if (isspace(c)) {
        return CharType::Whitespace;
    }
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=') return CharType::Operator_;
    if (c == '(' || c == ')') return CharType::Paren;
    return CharType::Other;
}

Token Lexer::getNextToken() {
    currentToken.clear();
    currentState = LexerState::Start;
    char c;

    while (input.get(c)) {
        column++;
        
        CharType type = getCharType(c);

        // Skip whitespace
        if (currentState == LexerState::Start && type == CharType::Whitespace) {
            continue;
        }

        // Handle parentheses as single tokens
        if (type == CharType::Paren) {
            if (!currentToken.empty()) {
                input.unget();
                column--;
                break;
            }
            currentToken = c;
            return Token(c == '(' ? TokenType::OpenParen : TokenType::CloseParen, currentToken, line, column);
        }

        LexerState next = transitionMatrix[(int)currentState][(int)type];
        
        // If we can't transition, stop building current token
        if (currentState != LexerState::Start && next == LexerState::Start) {
            input.unget();
            column--;
            break;
        }

        currentState = next;
        currentToken += c;

        // Operators are single characters
        if (currentState == LexerState::InOperator) {
            break;
        }
    }

    if (currentToken.empty()) return Token(TokenType::EndOfExpr, "", line, column);

    if (currentState == LexerState::InNumber) return Token(TokenType::Number, currentToken, line, column);
    if (currentState == LexerState::InName) return Token(TokenType::Name, currentToken, line, column);
    if (currentState == LexerState::InOperator) {
        return Token(currentToken == "=" ? TokenType::Assignment : TokenType::Operator, currentToken, line, column);
    }

    return Token(TokenType::Unknown, currentToken, line, column);
}