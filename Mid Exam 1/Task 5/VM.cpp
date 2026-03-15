#include "VM.h"
#include <iostream>

double VirtualMachine::execute(const std::vector<Instruction>& program, std::vector<double>& rv) {
    if (program.empty()) return 0.0;

    double lastResult = 0.0;
    for (const auto& instr : program) {
        std::cout << "[VM] ";
        switch (instr.op) {
            case OpCode::LOAD_CONST:
                rv[instr.resIdx] = instr.val;
                std::cout << "LOAD rv[" << instr.resIdx << "] = " << instr.val;
                break;
            case OpCode::ADD:
                rv[instr.resIdx] = rv[instr.left] + rv[instr.right];
                std::cout << "ADD rv[" << instr.resIdx << "] = rv[" << instr.left << "] + rv[" << instr.right << "]";
                break;
            case OpCode::SUB:
                rv[instr.resIdx] = rv[instr.left] - rv[instr.right];
                std::cout << "SUB rv[" << instr.resIdx << "] = rv[" << instr.left << "] - rv[" << instr.right << "]";
                break;
            case OpCode::MUL:
                rv[instr.resIdx] = rv[instr.left] * rv[instr.right];
                std::cout << "MUL rv[" << instr.resIdx << "] = rv[" << instr.left << "] * rv[" << instr.right << "]";
                break;
            case OpCode::DIV:
                if (rv[instr.right] == 0) throw std::runtime_error("Division by zero");
                rv[instr.resIdx] = rv[instr.left] / rv[instr.right];
                std::cout << "DIV rv[" << instr.resIdx << "] = rv[" << instr.left << "] / rv[" << instr.right << "]";
                break;
            default: break;
        }
        lastResult = rv[instr.resIdx];
        std::cout << " | Result: " << lastResult << std::endl;
    }
    return lastResult;
}