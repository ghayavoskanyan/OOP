#pragma once
#include "ASTNode.h"
#include "SymbolTable.h"
#include <memory>

class Calculator {
private:
    SymbolTable& symbolTable;
    
    double evaluateNode(ASTNode* node);
    
public:
    Calculator(SymbolTable& symTable);
    double calculate(std::unique_ptr<ASTNode> ast);
    void setVariable(const std::string& name, double value);
};
