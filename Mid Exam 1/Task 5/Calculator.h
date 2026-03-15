/* Calculator.h */
#pragma once
#include "ASTNode.h"
#include "SymbolTable.h"
#include <memory>

class Calculator {
private:
    SymbolTable& symbolTable;
public:
    Calculator(SymbolTable& st); 
    double calculate(std::unique_ptr<ASTNode> root);
};