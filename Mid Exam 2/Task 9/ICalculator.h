#pragma once
#include "ASTNode.h"
#include "SymbolTable.h"
#include "VM.h"
#include <vector>

class ICalculator {
public:
    virtual ~ICalculator() = default;
    virtual double calculate(const ASTNode* root) = 0;
};

class InstructionCalculator : public ICalculator {
private:
    SymbolTable& symbolTable;
    std::vector<Instruction> program;
    int nextReg;
    int getNewRegister();
    int compileNode(const ASTNode* node);
public:
    InstructionCalculator(SymbolTable& sym);
    double calculate(const ASTNode* root) override;
    const std::vector<Instruction>& getProgram() const;
    void clear();
};