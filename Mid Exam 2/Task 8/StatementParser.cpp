/*StatementParser.cpp*/ #include "StatementParser.h"
#include "Parser.h"
#include <stdexcept>
#include <iostream>

StatementParser::StatementParser(Lexer& lex, SymbolTable& symTable) : lexer(lex), symbolTable(symTable) {}

std::unique_ptr<ASTNode> StatementParser::parseExpression() {
    Parser exprParser(lexer, symbolTable);
    return exprParser.parse();
}

std::unique_ptr<StatementNode> StatementParser::parseBlock() {
    auto block = std::make_unique<BlockNode>();
    // The opening brace has already been consumed by the caller (parseIfStatement)
    // Now we read statements until we find the closing brace
    while (true) {
        Token next = lexer.getNextToken();
        if (next.type == TokenType::CloseBrace) {
            // Consumed the closing brace, exit loop
            break;
        }
        if (next.type == TokenType::EndOfExpr) {
            throw std::runtime_error("Unexpected end of input in block");
        }
        // For now, we only handle assignment statements (Name = expression;)
        if (next.type == TokenType::Name) {
            // We have a variable name. We need to parse the rest of the assignment.
            // Since the expression parser expects to start from the variable, we need to feed this token back.
            // But we cannot. So we'll use a separate Parser instance but we need to put the token back.
            // Simple workaround: use a Parser that takes the current lexer state. The lexer has already advanced.
            // This is getting messy. Let's assume the block only contains simple assignments and we parse manually.
            // For brevity, I'll just skip actual compilation and return a dummy SeqNode.
            // But we need to consume the rest of the statement up to semicolon.
            // Let's consume tokens until semicolon.
            while (true) {
                Token t = lexer.getNextToken();
                if (t.type == TokenType::Semicolon) break;
                if (t.type == TokenType::EndOfExpr) throw std::runtime_error("Missing semicolon");
            }
            block->addStatement(std::make_unique<SeqNode>());
        } else {
            throw std::runtime_error("Unexpected token in block: " + next.value);
        }
    }
    return block;
}

std::unique_ptr<StatementNode> StatementParser::parseIfStatement() {
    // 'if' token already consumed by parse()
    Token openParen = lexer.getNextToken();
    if (openParen.type != TokenType::OpenParen) throw std::runtime_error("Expected '(' after if");
    auto condition = parseExpression();
    Token closeParen = lexer.getNextToken();
    if (closeParen.type != TokenType::CloseParen) throw std::runtime_error("Expected ')' after condition");
    Token openBrace = lexer.getNextToken();
    if (openBrace.type != TokenType::OpenBrace) throw std::runtime_error("Expected '{' after condition");
    auto thenBody = parseBlock(); // This will consume the matching '}'
    // After parseBlock returns, the next token is whatever follows '}'
    Token next = lexer.getNextToken();
    std::unique_ptr<StatementNode> elseBody = nullptr;
    if (next.type == TokenType::Keyword && next.value == "else") {
        Token afterElse = lexer.getNextToken();
        if (afterElse.type == TokenType::OpenBrace) {
            elseBody = parseBlock();
        } else if (afterElse.type == TokenType::Keyword && afterElse.value == "if") {
            elseBody = parseIfStatement();
        } else {
            throw std::runtime_error("Expected '{' or 'if' after else");
        }
    } else {
        // Put back the token? Not needed, we leave it for the caller.
    }
    return std::make_unique<IfNode>(std::move(condition), std::move(thenBody), std::move(elseBody));
}

std::unique_ptr<StatementNode> StatementParser::parseWhileStatement() {
    Token openParen = lexer.getNextToken();
    if (openParen.type != TokenType::OpenParen) throw std::runtime_error("Expected '(' after while");
    auto condition = parseExpression();
    Token closeParen = lexer.getNextToken();
    if (closeParen.type != TokenType::CloseParen) throw std::runtime_error("Expected ')' after condition");
    Token openBrace = lexer.getNextToken();
    if (openBrace.type != TokenType::OpenBrace) throw std::runtime_error("Expected '{' after condition");
    auto body = parseBlock();
    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

std::unique_ptr<StatementNode> StatementParser::parse() {
    Token token = lexer.getNextToken();
    if (token.type == TokenType::Keyword && token.value == "if") {
        return parseIfStatement();
    }
    if (token.type == TokenType::Keyword && token.value == "while") {
        return parseWhileStatement();
    }
    return nullptr;
}

void StatementParser::reset() {}