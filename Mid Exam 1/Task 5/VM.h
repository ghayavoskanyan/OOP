#pragma once
#include <vector>
#include <iostream>
#include <stdexcept>

enum class OpCode { ADD, SUB, MUL, DIV, LOAD_CONST, HALT };

struct Instruction {
    OpCode op;
    double val;
    int left;
    int right;
    int resIdx;
};

class VirtualMachine {
public:
    double execute(const std::vector<Instruction>& program, std::vector<double>& rv);
};