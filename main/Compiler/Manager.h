#pragma once
#include "Lexer.h"
#include "StatementParser.h"
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
    std::unique_ptr<StatementParser> stmtParser;
    std::unique_ptr<InstructionCalculator> calculator;
    std::vector<std::unique_ptr<std::istringstream>> ownedStreams;

    void runProgram(const std::string& code, bool printResults);

public:
    Manager();
    
    SymbolTable& getSymbolTable() { return symbolTable; }
    const SymbolTable& getSymbolTable() const { return symbolTable; }
    
    void setInput(const std::string& expression);
    bool setInputFromFile(const std::string& filepath);
    double evaluate();
    void runFile(const std::string& filepath);
    bool getVariable(const std::string& name, double& value);
    void printAllVariables();
    void reset();
};