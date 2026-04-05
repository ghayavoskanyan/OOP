/*VM.cpp*/
#include "VM.h"
#include <iostream>

VirtualMachine::VirtualMachine(SymbolTable& sym, size_t regCount)
    : symbolTable(sym), values(sym.getValuesVector()), pc(0),
      zeroFlag(false), carryFlag(false), signedFlag(false) {
    rv.resize(regCount, 0.0);
}

void VirtualMachine::setRegister(size_t idx, double val) {
    if (idx < rv.size()) rv[idx] = val;
}

double VirtualMachine::getRegister(size_t idx) const {
    return (idx < rv.size()) ? rv[idx] : 0.0;
}

double VirtualMachine::execute(const std::vector<Instruction>& program) {
    if (program.empty()) return 0.0;
    pc = 0;
    double lastResult = 0.0;

    while (pc < program.size()) {
        const auto& instr = program[pc];
        std::cout << "[VM] PC=" << pc << " ";

        switch (instr.opcode) {
            case OpCode::LI: {
                auto& data = std::get<LiData>(instr.data);
                rv[data.dest] = (double)data.value;
                std::cout << "LI r" << (int)data.dest << ", " << data.value;
                lastResult = rv[data.dest];
                break;
            }
            case OpCode::LOAD: {
                auto& data = std::get<ArithData>(instr.data);
                rv[data.dest] = values[data.right];
                std::cout << "LOAD r" << (int)data.dest << ", [v" << (int)data.right << "]";
                lastResult = rv[data.dest];
                break;
            }
            case OpCode::STORE: {
                auto& data = std::get<ArithData>(instr.data);
                values[data.dest] = rv[data.right];
                std::cout << "STORE var[" << (int)data.dest << "], r" << (int)data.right;
                lastResult = values[data.dest];
                break;
            }
            case OpCode::ADD: {
                auto& data = std::get<ArithData>(instr.data);
                rv[data.dest] = rv[data.left] + rv[data.right];
                std::cout << "ADD r" << (int)data.dest << ", r" << (int)data.left << ", r" << (int)data.right;
                lastResult = rv[data.dest];
                break;
            }
            case OpCode::SUB: {
                auto& data = std::get<ArithData>(instr.data);
                rv[data.dest] = rv[data.left] - rv[data.right];
                std::cout << "SUB r" << (int)data.dest << ", r" << (int)data.left << ", r" << (int)data.right;
                lastResult = rv[data.dest];
                break;
            }
            case OpCode::MUL: {
                auto& data = std::get<ArithData>(instr.data);
                rv[data.dest] = rv[data.left] * rv[data.right];
                std::cout << "MUL r" << (int)data.dest << ", r" << (int)data.left << ", r" << (int)data.right;
                lastResult = rv[data.dest];
                break;
            }
            case OpCode::DIV: {
                auto& data = std::get<ArithData>(instr.data);
                if (rv[data.right] == 0) throw std::runtime_error("Division by zero");
                rv[data.dest] = rv[data.left] / rv[data.right];
                std::cout << "DIV r" << (int)data.dest << ", r" << (int)data.left << ", r" << (int)data.right;
                lastResult = rv[data.dest];
                break;
            }
            case OpCode::LIL: {
                auto& data = std::get<LiData>(instr.data);
                rv[data.dest] = (double)(data.value & 0xFFFF);
                std::cout << "LIL r" << (int)data.dest << ", " << (data.value & 0xFFFF);
                lastResult = rv[data.dest];
                break;
            }
            case OpCode::LIH: {
                auto& data = std::get<LiData>(instr.data);
                rv[data.dest] += ((double)((data.value >> 16) & 0xFFFF) * 65536.0);
                std::cout << "LIH r" << (int)data.dest << ", " << ((data.value >> 16) & 0xFFFF);
                lastResult = rv[data.dest];
                break;
            }
            case OpCode::CMP: {
                auto& data = std::get<ArithData>(instr.data);
                double diff = rv[data.left] - rv[data.right];
                zeroFlag = (diff == 0);
                carryFlag = (diff < 0);
                signedFlag = (diff < 0);
                std::cout << "CMP r" << (int)data.left << ", r" << (int)data.right;
                lastResult = diff;
                break;
            }
            case OpCode::JMP: {
                auto& data = std::get<ArithData>(instr.data);
                pc = data.dest - 1;
                std::cout << "JMP " << (int)data.dest;
                lastResult = 0;
                break;
            }
            case OpCode::JE: {
                auto& data = std::get<ArithData>(instr.data);
                if (zeroFlag) {
                    pc = data.dest - 1;
                    std::cout << "JE " << (int)data.dest;
                } else {
                    std::cout << "JE " << (int)data.dest << " (not taken)";
                }
                lastResult = 0;
                break;
            }
            case OpCode::JNE: {
                auto& data = std::get<ArithData>(instr.data);
                if (!zeroFlag) {
                    pc = data.dest - 1;
                    std::cout << "JNE " << (int)data.dest;
                } else {
                    std::cout << "JNE " << (int)data.dest << " (not taken)";
                }
                lastResult = 0;
                break;
            }
            case OpCode::JG: {
                auto& data = std::get<ArithData>(instr.data);
                if (!zeroFlag && !signedFlag) {
                    pc = data.dest - 1;
                    std::cout << "JG " << (int)data.dest;
                } else {
                    std::cout << "JG " << (int)data.dest << " (not taken)";
                }
                lastResult = 0;
                break;
            }
            case OpCode::JL: {
                auto& data = std::get<ArithData>(instr.data);
                if (signedFlag) {
                    pc = data.dest - 1;
                    std::cout << "JL " << (int)data.dest;
                } else {
                    std::cout << "JL " << (int)data.dest << " (not taken)";
                }
                lastResult = 0;
                break;
            }
            default:
                throw std::runtime_error("Unknown opcode");
        }
        std::cout << " = " << lastResult << "\n";
        pc++;
    }
    return lastResult;
}