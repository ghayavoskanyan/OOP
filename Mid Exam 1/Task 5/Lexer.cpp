#include "Lexer.h"
#include <cctype>

Lexer::Lexer(std::istream& is) : input(is), currentState(LexerState::Start), line(1), column(0), isEscape(false) {
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
    transitionMatrix[(int)LexerState::Start][(int)CharType::Quote] = LexerState::InString;
    
    transitionMatrix[(int)LexerState::InNumber][(int)CharType::Digit] = LexerState::InNumber;
    
    transitionMatrix[(int)LexerState::InName][(int)CharType::Letter] = LexerState::InName;
    transitionMatrix[(int)LexerState::InName][(int)CharType::Digit] = LexerState::InName;
}

Lexer::CharType Lexer::getCharType(char c) {
    if (isdigit(c)) return CharType::Digit;
    if (isalpha(c)) return CharType::Letter;
    if (isspace(c)) {
        if (c == '\n' || c == '\r') return CharType::Newline_;
        return CharType::Whitespace;
    }
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=') return CharType::Operator_;
    if (c == '(' || c == ')') return CharType::Paren;
    if (c == '"') return CharType::Quote;
    return CharType::Other;
}

Token Lexer::getNextToken() {
    currentToken.clear();
    currentState = LexerState::Start;
    char c;

    while (input.get(c)) {
        column++;
        if (c == '\n') { line++; column = 0; }

        CharType type = getCharType(c);

        if (currentState == LexerState::Start && (type == CharType::Whitespace || type == CharType::Newline_)) {
            continue;
        }

        if ((type == CharType::Paren || type == CharType::Operator_) && !currentToken.empty()) {
            input.unget(); column--;
            break;
        }

        if (type == CharType::Whitespace || type == CharType::Newline_) {
            break; 
        }

        if (type == CharType::Paren) {
            currentToken = c;
            return Token(c == '(' ? TokenType::OpenParen : TokenType::CloseParen, currentToken, line, column);
        }

        LexerState next = transitionMatrix[(int)currentState][(int)type];
        
        if (currentState != LexerState::Start && next != currentState) {
            input.unget(); column--;
            break;
        }

        currentState = next;
        currentToken += c;

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