#pragma once
#include "Lexer.h"
#include "Parser.h"
#include "Calculator.h"
#include "SymbolTable.h"
#include <memory>
#include <string>
#include <sstream>
#include <vector>

class Manager {
private:
    SymbolTable symbolTable;
    std::unique_ptr<Lexer> lexer;
    std::unique_ptr<Parser> parser;
    std::unique_ptr<Calculator> calculator;
    std::istream* currentInput;
    std::vector<std::unique_ptr<std::istringstream>> ownedStreams; 
    
public:
    Manager();
    void setInput(const std::string& expression);
    void setInput(std::istream& is);
    double evaluate();
    void setVariable(const std::string& name, double value);
    bool getVariable(const std::string& name, double& value);
    void reset(); 
};