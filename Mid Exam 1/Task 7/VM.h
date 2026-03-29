/*VM.h*/ #pragma once
#include <vector>
#include <stdexcept>
#include <variant>
#include "SymbolTable.h"

enum class OpCode : unsigned char {
    LI,   // Load Immediate (const -> r)
    LOAD, // Load Variable (var -> r) կամ Store (r -> var)
    STORE,
    ADD,
    SUB,
    MUL,
    DIV,
    LIL,  // Load Immediate Low
    LIH   // Load Immediate High
};

// MOV-ի տվյալներ (r -> r)
struct MovData {
    unsigned char src;
    unsigned char dst;
};

// Arithmetic-ի տվյալներ
struct ArithData {
    unsigned char dest;
    unsigned char left;
    unsigned char right;
};

// LI-ի տվյալներ (const արժեք)
struct LiData {
    unsigned char dest;
    unsigned int value;
};

using InstData = std::variant<MovData, ArithData, LiData>;

struct Instruction {
    OpCode opcode;
    InstData data;
    
    Instruction() : opcode(OpCode::LI), data(LiData{0, 0}) {}
    Instruction(OpCode op, MovData d) : opcode(op), data(d) {}
    Instruction(OpCode op, ArithData d) : opcode(op), data(d) {}
    Instruction(OpCode op, LiData d) : opcode(op), data(d) {}
};

class VirtualMachine {
    SymbolTable& symbolTable;
    std::vector<double> rv;
    std::vector<double>& values;
    size_t pc;
    
public:
    VirtualMachine(SymbolTable& sym, size_t regCount = 16);
    double execute(const std::vector<Instruction>& program);
    void setRegister(size_t idx, double val);
    double getRegister(size_t idx) const;
};