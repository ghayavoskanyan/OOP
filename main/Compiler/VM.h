#pragma once
#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <variant>
#include <vector>
#include "SymbolTable.h"

enum class OpCode : unsigned char {
    LI, LOAD, STORE,
    ADD, SUB, MUL, DIV,
    LIL, LIH,
    CMP, JMP, JE, JNE, JG, JL, JGE, JLE,
    PRINT
};

struct ArithData {
    uint32_t dest;
    uint32_t left;
    uint32_t right;
};

struct LiData {
    uint32_t dest;
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
    std::vector<int32_t> rv;
    std::vector<int32_t>& values;
    size_t pc;
    bool zeroFlag;
    bool carryFlag;
    bool signedFlag;

public:
    VirtualMachine(SymbolTable& sym, size_t regCount = 64);
    int32_t execute(const std::vector<Instruction>& program);
    void setRegister(size_t idx, int32_t val);
    int32_t getRegister(size_t idx) const;
};
