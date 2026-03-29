/*Manager.h*/ #pragma once
#include "Lexer.h"
#include "Parser.h"
#include "ICalculator.h"
#include "SymbolTable.h"
#include <memory>
#include <string>
#include <vector>
#include <sstream>

class Manager {
private:
    SymbolTable symbolTable;
    std::unique_ptr<Lexer> lexer;
    std::unique_ptr<Parser> parser;
    std::unique_ptr<InstructionCalculator> calculator;
    
    std::vector<std::unique_ptr<std::istringstream>> ownedStreams; 
    
public:
    Manager();
    void setInput(const std::string& expression);
    double evaluate();
    bool getVariable(const std::string& name, double& value);
    void reset(); 
};