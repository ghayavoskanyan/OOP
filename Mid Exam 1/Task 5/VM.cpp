#include "VM.h"
#include <iostream>
#include <algorithm>

double VirtualMachine::execute(const std::vector<Instruction>& program, std::vector<double>& rv) {
    if (program.empty()) return 0.0;

    size_t maxIdx = 0;
    for (const auto& instr : program) {
        if (instr.resIdx >= 0) maxIdx = std::max(maxIdx, (size_t)instr.resIdx);
        if (instr.left >= 0)   maxIdx = std::max(maxIdx, (size_t)instr.left);
        if (instr.right >= 0)  maxIdx = std::max(maxIdx, (size_t)instr.right);
    }
    rv.resize(maxIdx + 1, 0.0);

    double lastResult = 0.0;

    for (const auto& instr : program) {
        std::cout << "[VM] ";

        double leftVal  = (instr.left  >= 0) ? rv[instr.left]  : 0.0;
        double rightVal = (instr.right >= 0) ? rv[instr.right] : 0.0;

        switch (instr.op) {
            case OpCode::LOAD_CONST:
                rv[instr.resIdx] = instr.val;
                std::cout << "LOAD rv[" << instr.resIdx << "] = " << instr.val;
                break;

            case OpCode::ADD:
                rv[instr.resIdx] = leftVal + rightVal;
                std::cout << "ADD rv[" << instr.resIdx << "] = " << leftVal << " + " << rightVal;
                break;

            case OpCode::SUB:
                rv[instr.resIdx] = leftVal - rightVal;
                std::cout << "SUB rv[" << instr.resIdx << "] = " << leftVal << " - " << rightVal;
                break;

            case OpCode::MUL:
                rv[instr.resIdx] = leftVal * rightVal;
                std::cout << "MUL rv[" << instr.resIdx << "] = " << leftVal << " * " << rightVal;
                break;

            case OpCode::DIV:
                if (rightVal == 0) throw std::runtime_error("Division by zero");
                rv[instr.resIdx] = leftVal / rightVal;
                std::cout << "DIV rv[" << instr.resIdx << "] = " << leftVal << " / " << rightVal;
                break;

            default:
                break;
        }

        lastResult = rv[instr.resIdx];
        std::cout << " | Result: " << lastResult << std::endl;
    }

    return lastResult;
}