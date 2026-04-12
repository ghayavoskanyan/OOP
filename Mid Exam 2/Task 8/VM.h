#pragma once
#include <vector>
#include <stdexcept>
#include <variant>
#include "SymbolTable.h"

enum class OpCode : unsigned char {
    LI, LOAD, STORE,
    ADD, SUB, MUL, DIV,
    LIL, LIH,
    CMP, JMP, JE, JNE, JG, JL, JGE, JLE
};

struct ArithData {
    unsigned char dest;
    unsigned char left;
    unsigned char right;
};

struct LiData {
    unsigned char dest;
    unsigned int value;
};

using InstData = std::variant<ArithData, LiData>;

struct Instruction {
    OpCode opcode;
    InstData data;
    Instruction() : opcode(OpCode::LI), data(LiData{0, 0}) {}
    Instruction(OpCode op, ArithData d) : opcode(op), data(d) {}
    Instruction(OpCode op, LiData d) : opcode(op), data(d) {}
};

class VirtualMachine {
private:
    SymbolTable& symbolTable;
    std::vector<double> rv;
    std::vector<double>& values;
    size_t pc;
    bool zeroFlag;
    bool carryFlag;
    bool signedFlag;
public:
    VirtualMachine(SymbolTable& sym, size_t regCount = 64);
    double execute(const std::vector<Instruction>& program);
    void setRegister(size_t idx, double val);
    double getRegister(size_t idx) const;
};