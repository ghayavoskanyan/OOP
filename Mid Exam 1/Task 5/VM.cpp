#include "VM.h"
#include <iostream>
#include <algorithm>

double VirtualMachine::execute(const std::vector<Instruction>& program, std::vector<double>& rv) {
    if (program.empty()) return 0.0;

    // Ensure result vector is large enough
    size_t maxIdx = 0;
    for (const auto& instr : program) {
        maxIdx = std::max(maxIdx, (size_t)std::max({instr.resIdx, instr.left, instr.right}));
        if (instr.left >= 0) maxIdx = std::max(maxIdx, (size_t)instr.left);
        if (instr.right >= 0) maxIdx = std::max(maxIdx, (size_t)instr.right);
    }
    
    if (rv.size() <= maxIdx) {
        rv.resize(maxIdx + 1, 0.0);
    }

    double lastResult = 0.0;

    for (size_t i = 0; i < program.size(); i++) {
        const auto& instr = program[i];
        std::cout << "[VM] ";
        
        switch (instr.op) {
            case OpCode::LOAD_CONST:
                rv[instr.resIdx] = instr.val;
                std::cout << "LOAD rv[" << instr.resIdx << "] = " << instr.val;
                break;

            case OpCode::LOAD_VAR:
                rv[instr.resIdx] = symbolTable.getValueByIndex(instr.left);
                std::cout << "LOAD_VAR rv[" << instr.resIdx << "] = var[" << instr.left << "] (" 
                         << rv[instr.resIdx] << ")";
                break;

            case OpCode::STORE_VAR:
                rv[instr.resIdx] = rv[instr.left];
                symbolTable.setValueByIndex(instr.resIdx, rv[instr.left]);
                std::cout << "STORE var[" << instr.resIdx << "] = rv[" << instr.left << "] (" 
                         << rv[instr.left] << ")";
                break;

            case OpCode::ADD:
                rv[instr.resIdx] = rv[instr.left] + rv[instr.right];
                std::cout << "ADD rv[" << instr.left << "] + rv[" << instr.right << "]";
                break;

            case OpCode::SUB:
                rv[instr.resIdx] = rv[instr.left] - rv[instr.right];
                std::cout << "SUB rv[" << instr.left << "] - rv[" << instr.right << "]";
                break;

            case OpCode::MUL:
                rv[instr.resIdx] = rv[instr.left] * rv[instr.right];
                std::cout << "MUL rv[" << instr.left << "] * rv[" << instr.right << "]";
                break;

            case OpCode::DIV:
                if (rv[instr.right] == 0) {
                    throw std::runtime_error("Division by zero");
                }
                rv[instr.resIdx] = rv[instr.left] / rv[instr.right];
                std::cout << "DIV rv[" << instr.left << "] / rv[" << instr.right << "]";
                break;
        }

        lastResult = rv[instr.resIdx];
        std::cout << " = " << lastResult << std::endl;
    }

    return lastResult;
}