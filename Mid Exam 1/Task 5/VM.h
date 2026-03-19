#pragma once
#include <vector>
#include <stdexcept>
#include "SymbolTable.h"

enum class OpCode {
    LOAD_CONST,
    LOAD_VAR,
    STORE_VAR,
    ADD,
    SUB,
    MUL,
    DIV
};

struct Instruction {
    OpCode op;
    double val;
    int left;
    int right;
    int resIdx;
};

class VirtualMachine {
    SymbolTable& symbolTable;

public:
    VirtualMachine(SymbolTable& sym) : symbolTable(sym) {}

    double execute(const std::vector<Instruction>& program, std::vector<double>& rv);
};