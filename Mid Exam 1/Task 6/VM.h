#pragma once
#include <vector>
#include <stdexcept>
#include "SymbolTable.h"

enum class OpCode {
    MOV,
    ADD,
    SUB,
    MUL,
    DIV
};

enum class OperandType {
    REG,
    VAR,
    CONST
};

struct Instruction {
    OpCode op;
    union {
        struct {
            OperandType srcType;
            int srcIdx;
            OperandType dstType;
            int dstIdx;
        } mov;
        struct {
            int left;
            int right;
            int resIdx;
        } arith;
        double constVal; 
    } data;
    
    Instruction(OperandType srcT, int src, OperandType dstT, int dst) : op(OpCode::MOV) {
        data.mov.srcType = srcT;
        data.mov.srcIdx = src;
        data.mov.dstType = dstT;
        data.mov.dstIdx = dst;
    }
    
    Instruction(OpCode o, int l, int r, int res) : op(o) {
        data.arith.left = l;
        data.arith.right = r;
        data.arith.resIdx = res;
    }
};

class VirtualMachine {
    SymbolTable& symbolTable;

public:
    VirtualMachine(SymbolTable& sym) : symbolTable(sym) {}

    double execute(const std::vector<Instruction>& program, std::vector<double>& rv);
};