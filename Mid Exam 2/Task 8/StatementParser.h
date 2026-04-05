/*StatementParser.h*/ #pragma once
#include "Lexer.h"
#include "StatementNode.h"
#include "SymbolTable.h"
#include <memory>

class StatementParser {
private:
    Lexer& lexer;
    SymbolTable& symbolTable;
    
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<StatementNode> parseBlock();
    std::unique_ptr<StatementNode> parseIfStatement();
    std::unique_ptr<StatementNode> parseWhileStatement();
    
public:
    StatementParser(Lexer& lex, SymbolTable& symTable);
    std::unique_ptr<StatementNode> parse();
    void reset();
};