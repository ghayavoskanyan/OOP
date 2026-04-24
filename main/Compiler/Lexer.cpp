#include "Lexer.h"
#include <cctype>

Lexer::Lexer(std::istream& is) : input(is), currentState(LexerState::Start), line(1), column(0) {
    initializeTransitionMatrix();
    initializeKeywords();
}

void Lexer::initializeKeywords() {
    keywords["if"]       = TokenType::Keyword;
    keywords["else"]   = TokenType::Keyword;
    keywords["while"]  = TokenType::Keyword;
    keywords["do"]     = TokenType::Keyword;
    keywords["for"]    = TokenType::Keyword;
    keywords["print"]  = TokenType::Keyword;
    keywords["int"]    = TokenType::Keyword;
    keywords["enum"]   = TokenType::Keyword;
    keywords["struct"] = TokenType::Keyword;
    keywords["union"]  = TokenType::Keyword;
    keywords["class"]  = TokenType::Keyword;
    keywords["public"] = TokenType::Keyword;
    keywords["private"] = TokenType::Keyword;
    keywords["extern"] = TokenType::Keyword;
    keywords["var"]    = TokenType::Keyword;
    keywords["static"] = TokenType::Keyword;
    keywords["return"] = TokenType::Keyword;
    keywords["switch"] = TokenType::Keyword;
    keywords["case"]   = TokenType::Keyword;
    keywords["default"] = TokenType::Keyword;
    keywords["void"]   = TokenType::Keyword;
    keywords["break"]  = TokenType::Keyword;
    keywords["continue"] = TokenType::Keyword;
    keywords["goto"]   = TokenType::Keyword;
    keywords["static_cast"] = TokenType::Keyword;
}

void Lexer::initializeTransitionMatrix() {
    for (int i = 0; i < STATE_COUNT; i++)
        for (int j = 0; j < CHAR_TYPE_COUNT; j++)
            transitionMatrix[i][j] = LexerState::Start;

    transitionMatrix[(int)LexerState::Start][(int)CharType::Digit]    = LexerState::InNumber;
    transitionMatrix[(int)LexerState::Start][(int)CharType::Letter]   = LexerState::InName;
    transitionMatrix[(int)LexerState::Start][(int)CharType::Operator_]= LexerState::InOperator;

    transitionMatrix[(int)LexerState::InNumber][(int)CharType::Digit] = LexerState::InNumber;
    transitionMatrix[(int)LexerState::InNumber][(int)CharType::Dot]   = LexerState::InDecimal;

    transitionMatrix[(int)LexerState::InDecimal][(int)CharType::Digit]= LexerState::InDecimal;

    transitionMatrix[(int)LexerState::InName][(int)CharType::Letter]  = LexerState::InName;
    transitionMatrix[(int)LexerState::InName][(int)CharType::Digit]   = LexerState::InName;

    transitionMatrix[(int)LexerState::InOperator][(int)CharType::Operator_] = LexerState::InOperator;
}

Lexer::CharType Lexer::getCharType(char c) {
    if (isdigit(c))  return CharType::Digit;
    if (isalpha(c) || c == '_') return CharType::Letter;
    if (isspace(c))  return CharType::Whitespace;
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
        c == '=' || c == '>' || c == '<' || c == '!' || c == '&' || c == '|')
        return CharType::Operator_;
    if (c == '(' || c == ')') return CharType::Paren;
    if (c == '{' || c == '}') return CharType::Brace;
    if (c == ';') return CharType::Semicolon;
    if (c == '.') return CharType::Dot;
    return CharType::Other;
}

void Lexer::pushBack(const Token& token) {
    pendingTokens.push_back(token);
}

void Lexer::reset() {
    currentState = LexerState::Start;
    pendingTokens.clear();
    currentToken.clear();
    // Do not reset line/column because they are for error reporting only
}

Token Lexer::getNextToken() {
    if (!pendingTokens.empty()) {
        Token t = pendingTokens.back();
        pendingTokens.pop_back();
        return t;
    }

    currentToken.clear();
    currentState = LexerState::Start;
    char c;

    while (input.get(c)) {
        column++;
        if (c == '\n') { line++; column = 0; }

        CharType ct = getCharType(c);

        if (currentState == LexerState::Start && ct == CharType::Whitespace) continue;

        if (currentState == LexerState::Start && currentToken.empty() && (c == ':' || c == ',' || c == '.')) {
            if (c == '.')
                return Token(TokenType::Dot, ".", line, column);
            return Token(c == ':' ? TokenType::Colon : TokenType::Comma, std::string(1, c), line, column);
        }

        if (ct == CharType::Paren) {
            if (!currentToken.empty()) { input.unget(); column--; break; }
            currentToken = c;
            return Token(c == '(' ? TokenType::OpenParen : TokenType::CloseParen, currentToken, line, column);
        }
        if (ct == CharType::Brace) {
            if (!currentToken.empty()) { input.unget(); column--; break; }
            currentToken = c;
            return Token(c == '{' ? TokenType::OpenBrace : TokenType::CloseBrace, currentToken, line, column);
        }
        if (ct == CharType::Semicolon) {
            if (!currentToken.empty()) { input.unget(); column--; break; }
            currentToken = c;
            return Token(TokenType::Semicolon, currentToken, line, column);
        }

        if (currentState == LexerState::InOperator && ct != CharType::Operator_) {
            input.unget(); column--; break;
        }
        if (currentState == LexerState::InNumber && ct == CharType::Letter) {
            input.unget(); column--; break;
        }
        if ((currentState == LexerState::InNumber || currentState == LexerState::InDecimal) && ct == CharType::Operator_) {
            input.unget(); column--; break;
        }
        if (currentState == LexerState::InName && ct == CharType::Operator_) {
            input.unget(); column--; break;
        }
        if (currentState == LexerState::InName && ct == CharType::Whitespace) {
            break;
        }
        if (currentState == LexerState::InNumber && ct == CharType::Whitespace) {
            break;
        }

        LexerState next = transitionMatrix[(int)currentState][(int)ct];
        if (currentState != LexerState::Start && next == LexerState::Start) {
            input.unget(); column--; break;
        }
        currentState = next;
        currentToken += c;

        if (currentState == LexerState::InOperator) {
            char peek;
            if (input.get(peek)) {
                if (getCharType(peek) == CharType::Operator_) {
                    currentToken += peek; column++;
                } else {
                    input.unget();
                }
            }
            break;
        }
    }

    if (currentToken.empty()) return Token(TokenType::EndOfExpr, "", line, column);

    if (currentState == LexerState::InNumber || currentState == LexerState::InDecimal)
        return Token(TokenType::Number, currentToken, line, column);

    if (currentState == LexerState::InName) {
        auto it = keywords.find(currentToken);
        if (it != keywords.end())
            return Token(it->second, currentToken, line, column);
        return Token(TokenType::Name, currentToken, line, column);
    }

    if (currentState == LexerState::InOperator) {
        if (currentToken == "=")  return Token(TokenType::Assignment, currentToken, line, column);
        if (currentToken == "==" || currentToken == "!=" ||
            currentToken == ">=" || currentToken == "<=" ||
            currentToken == ">"  || currentToken == "<")
            return Token(TokenType::Comparison, currentToken, line, column);
        return Token(TokenType::Operator, currentToken, line, column);
    }

    return Token(TokenType::Unknown, currentToken, line, column);
}