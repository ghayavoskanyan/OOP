/*Lexer.cpp*/ 
#include "Lexer.h"
#include <cctype>

Lexer::Lexer(std::istream& is) : input(is), currentState(LexerState::Start), line(1), column(0) {
    initializeTransitionMatrix();
    initializeKeywords();
}

void Lexer::initializeKeywords() {
    keywords["if"] = TokenType::Keyword;
    keywords["else"] = TokenType::Keyword;
    keywords["while"] = TokenType::Keyword;
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
    if (isspace(c)) return CharType::Whitespace;
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '>' || c == '<' || c == '!') 
        return CharType::Operator_;
    if (c == '(' || c == ')') return CharType::Paren;
    if (c == '{' || c == '}') return CharType::Brace;
    if (c == ';') return CharType::Semicolon;
    return CharType::Other;
}

Token Lexer::getNextToken() {
    currentToken.clear();
    currentState = LexerState::Start;
    char c;

    while (input.get(c)) {
        column++;
        CharType type = getCharType(c);

        if (currentState == LexerState::Start && type == CharType::Whitespace) continue;

        if (type == CharType::Paren) {
            if (!currentToken.empty()) {
                input.unget(); column--; break;
            }
            currentToken = c;
            return Token(c == '(' ? TokenType::OpenParen : TokenType::CloseParen, currentToken, line, column);
        }
        
        if (type == CharType::Brace) {
            if (!currentToken.empty()) {
                input.unget(); column--; break;
            }
            currentToken = c;
            return Token(c == '{' ? TokenType::OpenBrace : TokenType::CloseBrace, currentToken, line, column);
        }
        
        if (type == CharType::Semicolon) {
            if (!currentToken.empty()) {
                input.unget(); column--; break;
            }
            currentToken = c;
            return Token(TokenType::Semicolon, currentToken, line, column);
        }

        LexerState next = transitionMatrix[(int)currentState][(int)type];
        if (currentState != LexerState::Start && next == LexerState::Start) {
            input.unget(); column--; break;
        }

        currentState = next;
        currentToken += c;

        if (currentState == LexerState::InOperator) break;
    }

    if (currentToken.empty()) return Token(TokenType::EndOfExpr, "", line, column);

    if (currentState == LexerState::InNumber) 
        return Token(TokenType::Number, currentToken, line, column);
    
    if (currentState == LexerState::InName) {
        auto it = keywords.find(currentToken);
        if (it != keywords.end()) 
            return Token(it->second, currentToken, line, column);
        return Token(TokenType::Name, currentToken, line, column);
    }
    
    if (currentState == LexerState::InOperator) {
        if (currentToken == "=") return Token(TokenType::Assignment, currentToken, line, column);
        if (currentToken == ">" || currentToken == "<" || currentToken == ">=" || 
            currentToken == "<=" || currentToken == "==" || currentToken == "!=")
            return Token(TokenType::Comparison, currentToken, line, column);
        return Token(TokenType::Operator, currentToken, line, column);
    }

    return Token(TokenType::Unknown, currentToken, line, column);
}