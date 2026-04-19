#include "StatementParser.h"
#include "ExprParser.h"
#include <sstream>
#include <stdexcept>

StatementParser::StatementParser(Lexer& lex, SymbolTable& symTable) : lexer(lex), symbolTable(symTable) {}

std::unique_ptr<ASTNode> StatementParser::parseExpression(bool stopAtCloseParen) {
    return expr_parser::parseExpression(lexer, symbolTable, stopAtCloseParen);
}

std::unique_ptr<StatementNode> StatementParser::parseDeclaration(bool isStatic) {
    Token nameTok = lexer.getNextToken();
    if (nameTok.type != TokenType::Name && nameTok.type != TokenType::Keyword) {
        throw std::runtime_error("Expected variable name after 'int' or 'var', got '" + nameTok.value + "'");
    }
    std::string varName = nameTok.value;

    Token next = lexer.getNextToken();
    std::unique_ptr<ASTNode> init = nullptr;
    if (next.type == TokenType::Assignment) {
        init = parseExpression(false);
        next = lexer.getNextToken();
    }
    if (next.type != TokenType::Semicolon)
        throw std::runtime_error("Expected ';' after variable declaration");

    return std::make_unique<DeclNode>(varName, std::move(init), symbolTable, isStatic);
}

std::unique_ptr<StatementNode> StatementParser::parseAfterIntType(bool isStatic) {
    Token name = lexer.getNextToken();
    if (name.type != TokenType::Name)
        throw std::runtime_error("Expected name after 'int'");

    Token la = lexer.getNextToken();
    if (la.type == TokenType::OpenParen) {
        lexer.pushBack(la);
        return parseFunctionDefinition(false, name.value);
    }
    lexer.pushBack(la);

    Token next = lexer.getNextToken();
    std::unique_ptr<ASTNode> init = nullptr;
    if (next.type == TokenType::Assignment) {
        init = parseExpression(false);
        next = lexer.getNextToken();
    }
    if (next.type != TokenType::Semicolon)
        throw std::runtime_error("Expected ';' after variable declaration");

    return std::make_unique<DeclNode>(name.value, std::move(init), symbolTable, isStatic);
}

std::unique_ptr<StatementNode> StatementParser::parseFunctionDefinition(bool voidReturn, const std::string& name) {
    Token op = lexer.getNextToken();
    if (op.type != TokenType::OpenParen) throw std::runtime_error("Expected '(' after function name");

    std::vector<std::pair<std::string, std::string>> params;
    while (true) {
        Token t = lexer.getNextToken();
        if (t.type == TokenType::CloseParen) break;
        if (t.type != TokenType::Keyword || t.value != "int")
            throw std::runtime_error("Expected 'int' or ')' in parameter list");
        Token pname = lexer.getNextToken();
        if (pname.type != TokenType::Name)
            throw std::runtime_error("Expected parameter name");
        params.push_back({"int", pname.value});
        Token t2 = lexer.getNextToken();
        if (t2.type == TokenType::CloseParen) break;
        if (t2.type != TokenType::Comma) throw std::runtime_error("Expected ',' or ')' in parameter list");
    }

    Token ob = lexer.getNextToken();
    if (ob.type != TokenType::OpenBrace) throw std::runtime_error("Expected '{' before function body");

    auto body = parseBlock();
    return std::make_unique<FunctionDefNode>(name, voidReturn, std::move(params), std::move(body));
}

std::unique_ptr<StatementNode> StatementParser::parseReturnStatement() {
    Token t = lexer.getNextToken();
    if (t.type == TokenType::Semicolon) return std::make_unique<ReturnNode>(nullptr, true);
    lexer.pushBack(t);
    auto e = parseExpression(false);
    Token semi = lexer.getNextToken();
    if (semi.type != TokenType::Semicolon) throw std::runtime_error("Expected ';' after return");
    return std::make_unique<ReturnNode>(std::move(e), false);
}

std::vector<std::unique_ptr<StatementNode>> StatementParser::parseCaseStatements() {
    std::vector<std::unique_ptr<StatementNode>> out;
    while (true) {
        Token t = lexer.getNextToken();
        if (t.type == TokenType::CloseBrace) {
            lexer.pushBack(t);
            break;
        }
        if (t.type == TokenType::Keyword && (t.value == "case" || t.value == "default")) {
            lexer.pushBack(t);
            break;
        }
        lexer.pushBack(t);
        auto st = parseStatement();
        if (!st) break;
        out.push_back(std::move(st));
    }
    return out;
}

std::unique_ptr<StatementNode> StatementParser::parseSwitchStatement() {
    Token op = lexer.getNextToken();
    if (op.type != TokenType::OpenParen) throw std::runtime_error("Expected '(' after switch");

    auto disc = parseExpression(true);

    Token cp = lexer.getNextToken();
    if (cp.type != TokenType::CloseParen) throw std::runtime_error("Expected ')' after switch expression");

    Token ob = lexer.getNextToken();
    if (ob.type != TokenType::OpenBrace) throw std::runtime_error("Expected '{' after switch(...)");

    std::vector<SwitchArm> arms;
    while (true) {
        Token t = lexer.getNextToken();
        if (t.type == TokenType::CloseBrace) break;

        if (t.type == TokenType::Keyword && t.value == "case") {
            Token num = lexer.getNextToken();
            if (num.type != TokenType::Number) throw std::runtime_error("Expected constant after case");
            Token col = lexer.getNextToken();
            if (col.type != TokenType::Colon) throw std::runtime_error("Expected ':' after case value");
            SwitchArm arm;
            arm.isDefault = false;
            arm.caseValue = static_cast<int32_t>(std::stol(num.value));
            arm.statements = parseCaseStatements();
            arms.push_back(std::move(arm));
            continue;
        }
        if (t.type == TokenType::Keyword && t.value == "default") {
            Token col = lexer.getNextToken();
            if (col.type != TokenType::Colon) throw std::runtime_error("Expected ':' after default");
            SwitchArm arm;
            arm.isDefault = true;
            arm.caseValue = 0;
            arm.statements = parseCaseStatements();
            arms.push_back(std::move(arm));
            continue;
        }
        throw std::runtime_error("Expected 'case', 'default', or '}' inside switch");
    }

    return std::make_unique<SwitchNode>(std::move(disc), std::move(arms));
}

std::unique_ptr<StatementNode> StatementParser::parseBreakStatement() {
    Token semi = lexer.getNextToken();
    if (semi.type != TokenType::Semicolon) throw std::runtime_error("Expected ';' after break");
    return std::make_unique<BreakNode>();
}

std::unique_ptr<StatementNode> StatementParser::parseBlock() {
    auto block = std::make_unique<BlockNode>();
    while (true) {
        Token t = lexer.getNextToken();
        if (t.type == TokenType::CloseBrace) break;
        if (t.type == TokenType::EndOfExpr)
            throw std::runtime_error("Unexpected end of input inside block");

        if (t.type == TokenType::Keyword) {
            if (t.value == "if") {
                block->addStatement(parseIfStatement());
            } else if (t.value == "while") {
                block->addStatement(parseWhileStatement());
            } else if (t.value == "for") {
                block->addStatement(parseForStatement());
            } else if (t.value == "print") {
                block->addStatement(parsePrintStatement());
            } else if (t.value == "static") {
                Token it = lexer.getNextToken();
                if (it.type != TokenType::Keyword || it.value != "int")
                    throw std::runtime_error("Expected 'int' after static");
                block->addStatement(parseAfterIntType(true));
            } else if (t.value == "int") {
                block->addStatement(parseAfterIntType(false));
            } else if (t.value == "var") {
                block->addStatement(parseDeclaration(false));
            } else if (t.value == "void") {
                Token name = lexer.getNextToken();
                if (name.type != TokenType::Name) throw std::runtime_error("Expected function name after void");
                block->addStatement(parseFunctionDefinition(true, name.value));
            } else if (t.value == "return") {
                block->addStatement(parseReturnStatement());
            } else if (t.value == "switch") {
                block->addStatement(parseSwitchStatement());
            } else if (t.value == "break") {
                block->addStatement(parseBreakStatement());
            } else {
                throw std::runtime_error("Unexpected keyword in block: '" + t.value + "'");
            }
        } else if (t.type == TokenType::Name || t.type == TokenType::Number || t.type == TokenType::Operator ||
                   t.type == TokenType::OpenParen) {
            lexer.pushBack(t);
            auto expr = parseExpression(false);
            Token semi = lexer.getNextToken();
            if (semi.type != TokenType::Semicolon)
                throw std::runtime_error("Expected ';' after expression");
            block->addStatement(std::make_unique<ExprStatement>(std::move(expr)));
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
    if (op.type != TokenType::OpenParen) throw std::runtime_error("Expected '(' after if");

    auto condition = parseExpression(true);

    Token cp = lexer.getNextToken();
    if (cp.type != TokenType::CloseParen) throw std::runtime_error("Expected ')' after if condition");

    Token ob = lexer.getNextToken();
    if (ob.type != TokenType::OpenBrace) throw std::runtime_error("Expected '{' after if(...)");

    auto thenBody = parseBlock();

    Token next = lexer.getNextToken();
    std::unique_ptr<StatementNode> elseBody = nullptr;

    if (next.type == TokenType::Keyword && next.value == "else") {
        Token afterElse = lexer.getNextToken();
        if (afterElse.type == TokenType::OpenBrace) {
            elseBody = parseBlock();
        } else if (afterElse.type == TokenType::Keyword && afterElse.value == "if") {
            lexer.pushBack(afterElse);
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
    if (op.type != TokenType::OpenParen) throw std::runtime_error("Expected '(' after while");

    auto condition = parseExpression(true);

    Token cp = lexer.getNextToken();
    if (cp.type != TokenType::CloseParen) throw std::runtime_error("Expected ')' after while condition");

    Token ob = lexer.getNextToken();
    if (ob.type != TokenType::OpenBrace) throw std::runtime_error("Expected '{' after while(...)");

    auto body = parseBlock();
    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

std::unique_ptr<StatementNode> StatementParser::parseForStatement() {
    Token op = lexer.getNextToken();
    if (op.type != TokenType::OpenParen) throw std::runtime_error("Expected '(' after for");

    auto init = parseExpression(false);
    Token semi1 = lexer.getNextToken();
    if (semi1.type != TokenType::Semicolon) throw std::runtime_error("Expected ';' after for init");

    auto condition = parseExpression(false);
    Token semi2 = lexer.getNextToken();
    if (semi2.type != TokenType::Semicolon) throw std::runtime_error("Expected ';' after for condition");

    auto update = parseExpression(true);
    Token cp = lexer.getNextToken();
    if (cp.type != TokenType::CloseParen) throw std::runtime_error("Expected ')' after for update");

    Token ob = lexer.getNextToken();
    if (ob.type != TokenType::OpenBrace) throw std::runtime_error("Expected '{' after for(...)");

    auto body = parseBlock();
    return std::make_unique<ForNode>(std::move(init), std::move(condition), std::move(update), std::move(body));
}

std::unique_ptr<StatementNode> StatementParser::parsePrintStatement() {
    Token op = lexer.getNextToken();
    if (op.type != TokenType::OpenParen) throw std::runtime_error("Expected '(' after print");

    auto expr = parseExpression(true);

    Token cp = lexer.getNextToken();
    if (cp.type != TokenType::CloseParen) throw std::runtime_error("Expected ')' after print(...)");

    Token semi = lexer.getNextToken();
    if (semi.type != TokenType::Semicolon) throw std::runtime_error("Expected ';' after print(...)");

    return std::make_unique<PrintNode>(std::move(expr), symbolTable);
}

std::unique_ptr<StatementNode> StatementParser::parseStatement() {
    Token t = lexer.getNextToken();
    if (t.type == TokenType::EndOfExpr) return nullptr;

    if (t.type == TokenType::Keyword) {
        if (t.value == "if") return parseIfStatement();
        if (t.value == "while") return parseWhileStatement();
        if (t.value == "for") return parseForStatement();
        if (t.value == "print") return parsePrintStatement();
        if (t.value == "static") {
            Token it = lexer.getNextToken();
            if (it.type != TokenType::Keyword || it.value != "int")
                throw std::runtime_error("Expected 'int' after static");
            return parseAfterIntType(true);
        }
        if (t.value == "int") return parseAfterIntType(false);
        if (t.value == "var") return parseDeclaration(false);
        if (t.value == "void") {
            Token name = lexer.getNextToken();
            if (name.type != TokenType::Name) throw std::runtime_error("Expected function name after void");
            return parseFunctionDefinition(true, name.value);
        }
        if (t.value == "return") return parseReturnStatement();
        if (t.value == "switch") return parseSwitchStatement();
        if (t.value == "break") return parseBreakStatement();
    }

    if (t.type == TokenType::Name || t.type == TokenType::Number || t.type == TokenType::Operator ||
        t.type == TokenType::OpenParen) {
        lexer.pushBack(t);
        auto expr = parseExpression(false);
        Token semi = lexer.getNextToken();
        if (semi.type != TokenType::Semicolon && semi.type != TokenType::EndOfExpr)
            throw std::runtime_error("Expected ';' after expression statement");
        return std::make_unique<ExprStatement>(std::move(expr));
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
    return seq;
}

void StatementParser::reset() {}
