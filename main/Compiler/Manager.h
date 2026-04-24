#pragma once
#include "Lexer.h"
#include "StatementParser.h"
#include "ICalculator.h"
#include "SymbolTable.h"
#include "TypeRegistry.h"
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

class Manager {
private:
    SymbolTable symbolTable;
    TypeRegistry typeRegistry;
    std::unique_ptr<Lexer> lexer;
    std::unique_ptr<StatementParser> stmtParser;
    std::unique_ptr<InstructionCalculator> calculator;
    std::vector<std::unique_ptr<std::istringstream>> ownedStreams;

public:
    Manager();

    SymbolTable& getSymbolTable() { return symbolTable; }
    const SymbolTable& getSymbolTable() const { return symbolTable; }

    void setInput(const std::string& expression);
    bool setInputFromFile(const std::string& filepath);
    int32_t evaluate();
    void runFile(const std::string& filepath);
    bool getVariable(const std::string& name, int32_t& value);
    void printAllVariables();
    void reset();

    bool compileFileToIr(const std::string& sourcePath, const std::string& irPath);
    bool compileFileToExe(const std::string& sourcePath, const std::string& exePath);
    bool runRiscvExe(const std::string& exePath, std::string& errOut);
};
