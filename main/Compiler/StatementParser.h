#pragma once
#include "Lexer.h"
#include "StatementNode.h"
#include "SymbolTable.h"
#include <memory>

class StatementParser {
private:
    Lexer& lexer;
    SymbolTable& symbolTable;

    std::unique_ptr<ASTNode> parseExpression(bool stopAtCloseParen = false);

    std::unique_ptr<StatementNode> parseBlock();
    std::unique_ptr<StatementNode> parseIfStatement();
    std::unique_ptr<StatementNode> parseWhileStatement();
    std::unique_ptr<StatementNode> parseForStatement();
    std::unique_ptr<StatementNode> parsePrintStatement();
    std::unique_ptr<StatementNode> parseDeclaration(bool isStatic);
    std::unique_ptr<StatementNode> parseStatement();
    std::unique_ptr<StatementNode> parseAfterIntType(bool isStatic);
    std::unique_ptr<StatementNode> parseFunctionDefinition(bool voidReturn, const std::string& name);
    std::unique_ptr<StatementNode> parseReturnStatement();
    std::unique_ptr<StatementNode> parseSwitchStatement();
    std::unique_ptr<StatementNode> parseBreakStatement();
    std::vector<std::unique_ptr<StatementNode>> parseCaseStatements();

public:
    StatementParser(Lexer& lex, SymbolTable& symTable);
    std::unique_ptr<StatementNode> parse();
    void reset();
};
