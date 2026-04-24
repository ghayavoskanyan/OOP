#pragma once
#include "Lexer.h"
#include "StatementNode.h"
#include "SymbolTable.h"
#include "TypeRegistry.h"
#include <memory>

class StatementParser {
private:
    Lexer& lexer;
    SymbolTable& symbolTable;
    TypeRegistry& typeRegistry;

    std::unique_ptr<ASTNode> parseExpression(bool stopAtCloseParen = false);

    std::unique_ptr<StatementNode> parseBlock();
    std::unique_ptr<StatementNode> parseIfStatement();
    std::unique_ptr<StatementNode> parseWhileStatement();
    std::unique_ptr<StatementNode> parseDoWhileStatement();
    std::unique_ptr<StatementNode> parseForStatement();
    std::unique_ptr<StatementNode> parsePrintStatement();
    std::unique_ptr<StatementNode> parseDeclaration(bool isStatic);
    std::unique_ptr<StatementNode> parseStatement();
    std::unique_ptr<StatementNode> parseAfterIntType(bool isStatic);
    std::unique_ptr<StatementNode> parseFunctionDefinition(bool voidReturn, const std::string& name);
    std::unique_ptr<StatementNode> parseReturnStatement();
    std::unique_ptr<StatementNode> parseSwitchStatement();
    std::unique_ptr<StatementNode> parseBreakStatement();
    std::unique_ptr<StatementNode> parseContinueStatement();
    std::unique_ptr<StatementNode> parseGotoStatement();
    std::unique_ptr<StatementNode> parseEnumDefinition();
    std::unique_ptr<StatementNode> parseAggregateDefinition(AggregateKind kind);
    std::unique_ptr<StatementNode> parseAggregateInstanceDeclaration(const std::string& typeName);
    std::vector<std::unique_ptr<StatementNode>> parseCaseStatements();

public:
    StatementParser(Lexer& lex, SymbolTable& symTable, TypeRegistry& types);
    std::unique_ptr<StatementNode> parse();
    void reset();
};
