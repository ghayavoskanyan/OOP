#include "StatementParser.h"
#include "Parser.h"
#include <stdexcept>
#include <sstream>

StatementParser::StatementParser(Lexer& lex, SymbolTable& symTable)
    : lexer(lex), symbolTable(symTable) {}

std::unique_ptr<ASTNode> StatementParser::parseExpression(bool stopAtCloseParen) {
    Parser exprParser(lexer, symbolTable);
    return exprParser.parse(stopAtCloseParen);
}

std::unique_ptr<StatementNode> StatementParser::parseBlock() {
    auto block = std::make_unique<BlockNode>();
    while (true) {
        Token t = lexer.getNextToken();
        if (t.type == TokenType::CloseBrace) break;
        if (t.type == TokenType::EndOfExpr)
            throw std::runtime_error("Unexpected end of input inside block");

        if (t.type == TokenType::Keyword && t.value == "if") {
            block->addStatement(parseIfStatement());
        } else if (t.type == TokenType::Keyword && t.value == "while") {
            block->addStatement(parseWhileStatement());
        } else if (t.type == TokenType::Keyword && t.value == "for") {
            block->addStatement(parseForStatement());
        } else if (t.type == TokenType::Keyword && t.value == "print") {
            block->addStatement(parsePrintStatement());
        } else if (t.type == TokenType::Name || t.type == TokenType::Number ||
                   t.type == TokenType::Operator) {
            std::string fullExpr = t.value;
            int depth = 0;
            while (true) {
                Token nx = lexer.getNextToken();
                if (nx.type == TokenType::OpenBrace)  depth++;
                if (nx.type == TokenType::CloseBrace) { if (depth == 0) { break; } depth--; }
                if ((nx.type == TokenType::Semicolon || nx.type == TokenType::EndOfExpr) && depth == 0) break;
                fullExpr += " " + nx.value;
            }

            std::istringstream ss(fullExpr);
            Lexer tmpLex(ss);
            Parser tmpParser(tmpLex, symbolTable);
            auto expr = tmpParser.parse(false);
            if (expr) block->addStatement(std::make_unique<ExprStatement>(std::move(expr)));
        } else if (t.type == TokenType::Semicolon) {
            continue;
        } else {
            throw std::runtime_error("Unexpected token in block: '" + t.value + "'");
        }
    }
    return block;
}

std::unique_ptr<StatementNode> StatementParser::parseIfStatement() {
    Token op = lexer.getNextToken();
    if (op.type != TokenType::OpenParen)
        throw std::runtime_error("Expected '(' after if");

    auto condition = parseExpression(true);

    Token cp = lexer.getNextToken();
    if (cp.type != TokenType::CloseParen)
        throw std::runtime_error("Expected ')' after if condition");

    Token ob = lexer.getNextToken();
    if (ob.type != TokenType::OpenBrace)
        throw std::runtime_error("Expected '{' after if(...)");

    auto thenBody = parseBlock();

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
        lexer.pushBack(next);
    }

    return std::make_unique<IfNode>(std::move(condition), std::move(thenBody), std::move(elseBody));
}

std::unique_ptr<StatementNode> StatementParser::parseWhileStatement() {
    Token op = lexer.getNextToken();
    if (op.type != TokenType::OpenParen)
        throw std::runtime_error("Expected '(' after while");

    auto condition = parseExpression(true);

    Token cp = lexer.getNextToken();
    if (cp.type != TokenType::CloseParen)
        throw std::runtime_error("Expected ')' after while condition");

    Token ob = lexer.getNextToken();
    if (ob.type != TokenType::OpenBrace)
        throw std::runtime_error("Expected '{' after while(...)");

    auto body = parseBlock();
    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

std::unique_ptr<StatementNode> StatementParser::parseForStatement() {
    Token op = lexer.getNextToken();
    if (op.type != TokenType::OpenParen)
        throw std::runtime_error("Expected '(' after for");

    auto init = parseExpression(false);
    Token semi1 = lexer.getNextToken();
    if (semi1.type != TokenType::Semicolon && semi1.type != TokenType::CloseParen)
        throw std::runtime_error("Expected ';' after for init");

    auto condition = parseExpression(false);
    Token semi2 = lexer.getNextToken();
    if (semi2.type != TokenType::Semicolon && semi2.type != TokenType::CloseParen)
        throw std::runtime_error("Expected ';' after for condition");

    auto update = parseExpression(false);
    Token cp = lexer.getNextToken();
    if (cp.type != TokenType::CloseParen)
        throw std::runtime_error("Expected ')' after for update");

    Token ob = lexer.getNextToken();
    if (ob.type != TokenType::OpenBrace)
        throw std::runtime_error("Expected '{' after for(...)");

    auto body = parseBlock();
    return std::make_unique<ForNode>(std::move(init), std::move(condition), std::move(update), std::move(body));
}

std::unique_ptr<StatementNode> StatementParser::parsePrintStatement() {
    Token op = lexer.getNextToken();
    if (op.type != TokenType::OpenParen)
        throw std::runtime_error("Expected '(' after print");

    // Parse expression and stop at the closing ')'
    auto expr = parseExpression(true);

    Token cp = lexer.getNextToken();
    if (cp.type != TokenType::CloseParen)
        throw std::runtime_error("Expected ')' after print(...)");

    Token semi = lexer.getNextToken();
    if (semi.type != TokenType::Semicolon)
        throw std::runtime_error("Expected ';' after print(...)");

    return std::make_unique<PrintNode>(std::move(expr), symbolTable);
}

std::unique_ptr<StatementNode> StatementParser::parseStatement() {
    Token t = lexer.getNextToken();
    if (t.type == TokenType::EndOfExpr) return nullptr;

    if (t.type == TokenType::Keyword) {
        if (t.value == "if")    return parseIfStatement();
        if (t.value == "while") return parseWhileStatement();
        if (t.value == "for")   return parseForStatement();
        if (t.value == "print") return parsePrintStatement();
    }

    if (t.type == TokenType::Name || t.type == TokenType::Number ||
        t.type == TokenType::Operator) {
        std::string fullExpr = t.value;
        while (true) {
            Token nx = lexer.getNextToken();
            if (nx.type == TokenType::Semicolon || nx.type == TokenType::EndOfExpr) break;
            fullExpr += " " + nx.value;
        }
        std::istringstream ss(fullExpr);
        Lexer tmpLex(ss);
        Parser tmpParser(tmpLex, symbolTable);
        auto expr = tmpParser.parse(false);
        if (expr) return std::make_unique<ExprStatement>(std::move(expr));
    }

    if (t.type == TokenType::Semicolon) return std::make_unique<SeqNode>();

    return nullptr;
}

std::unique_ptr<StatementNode> StatementParser::parse() {
    auto seq = std::make_unique<SeqNode>();
    while (true) {
        auto stmt = parseStatement();
        if (!stmt) break;
        seq->addStatement(std::move(stmt));
    }
    if (seq) return seq;
    return nullptr;
}

void StatementParser::reset() {}