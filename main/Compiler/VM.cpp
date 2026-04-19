#include "VM.h"
#include <iostream>

VirtualMachine::VirtualMachine(SymbolTable& sym, size_t regCount)
    : symbolTable(sym), values(sym.getValuesVector()), pc(0),
      zeroFlag(false), carryFlag(false), signedFlag(false) {
    rv.resize(regCount, 0);
}

void VirtualMachine::setRegister(size_t idx, int32_t val) {
    if (idx >= rv.size()) rv.resize(idx + 1, 0);
    rv[idx] = val;
}

int32_t VirtualMachine::getRegister(size_t idx) const {
    return (idx < rv.size()) ? rv[idx] : 0;
}

int32_t VirtualMachine::execute(const std::vector<Instruction>& program) {
    if (program.empty()) return 0;
    pc = 0;
    int32_t lastResult = 0;

    while (pc < program.size()) {
        const auto& instr = program[pc];

        switch (instr.opcode) {
            case OpCode::LI: {
                auto& d = std::get<LiData>(instr.data);
                if (static_cast<size_t>(d.dest) >= rv.size()) rv.resize(static_cast<size_t>(d.dest) + 1, 0);
                rv[static_cast<size_t>(d.dest)] = static_cast<int32_t>(d.value);
                lastResult = rv[static_cast<size_t>(d.dest)];
                break;
            }
            case OpCode::LIL: {
                auto& d = std::get<LiData>(instr.data);
                if (static_cast<size_t>(d.dest) >= rv.size()) rv.resize(static_cast<size_t>(d.dest) + 1, 0);
                rv[static_cast<size_t>(d.dest)] = static_cast<int32_t>(d.value & 0xFFFF);
                lastResult = rv[static_cast<size_t>(d.dest)];
                break;
            }
            case OpCode::LIH: {
                auto& d = std::get<LiData>(instr.data);
                if (static_cast<size_t>(d.dest) >= rv.size()) rv.resize(static_cast<size_t>(d.dest) + 1, 0);
                rv[static_cast<size_t>(d.dest)] += static_cast<int32_t>((d.value & 0xFFFF) * 65536);
                lastResult = rv[static_cast<size_t>(d.dest)];
                break;
            }
            case OpCode::LOAD: {
                auto& d = std::get<ArithData>(instr.data);
                if (static_cast<size_t>(d.dest) >= rv.size()) rv.resize(static_cast<size_t>(d.dest) + 1, 0);
                while (static_cast<size_t>(d.right) >= values.size()) values.push_back(0);
                rv[static_cast<size_t>(d.dest)] = values[static_cast<size_t>(d.right)];
                lastResult = rv[static_cast<size_t>(d.dest)];
                break;
            }
            case OpCode::STORE: {
                auto& d = std::get<ArithData>(instr.data);
                while (static_cast<size_t>(d.dest) >= values.size()) values.push_back(0);
                if (static_cast<size_t>(d.right) >= rv.size()) rv.resize(static_cast<size_t>(d.right) + 1, 0);
                values[static_cast<size_t>(d.dest)] = rv[static_cast<size_t>(d.right)];
                lastResult = values[static_cast<size_t>(d.dest)];
                break;
            }
            case OpCode::ADD: {
                auto& d = std::get<ArithData>(instr.data);
                if (static_cast<size_t>(d.dest) >= rv.size()) rv.resize(static_cast<size_t>(d.dest) + 1, 0);
                rv[static_cast<size_t>(d.dest)] = rv[static_cast<size_t>(d.left)] + rv[static_cast<size_t>(d.right)];
                lastResult = rv[static_cast<size_t>(d.dest)];
                break;
            }
            case OpCode::SUB: {
                auto& d = std::get<ArithData>(instr.data);
                if (static_cast<size_t>(d.dest) >= rv.size()) rv.resize(static_cast<size_t>(d.dest) + 1, 0);
                rv[static_cast<size_t>(d.dest)] = rv[static_cast<size_t>(d.left)] - rv[static_cast<size_t>(d.right)];
                lastResult = rv[static_cast<size_t>(d.dest)];
                break;
            }
            case OpCode::MUL: {
                auto& d = std::get<ArithData>(instr.data);
                if (static_cast<size_t>(d.dest) >= rv.size()) rv.resize(static_cast<size_t>(d.dest) + 1, 0);
                rv[static_cast<size_t>(d.dest)] = rv[static_cast<size_t>(d.left)] * rv[static_cast<size_t>(d.right)];
                lastResult = rv[static_cast<size_t>(d.dest)];
                break;
            }
            case OpCode::DIV: {
                auto& d = std::get<ArithData>(instr.data);
                if (rv[static_cast<size_t>(d.right)] == 0) throw std::runtime_error("Division by zero");
                if (static_cast<size_t>(d.dest) >= rv.size()) rv.resize(static_cast<size_t>(d.dest) + 1, 0);
                rv[static_cast<size_t>(d.dest)] = rv[static_cast<size_t>(d.left)] / rv[static_cast<size_t>(d.right)];
                lastResult = rv[static_cast<size_t>(d.dest)];
                break;
            }
            case OpCode::CMP: {
                auto& d = std::get<ArithData>(instr.data);
                int32_t diff = rv[static_cast<size_t>(d.left)] - rv[static_cast<size_t>(d.right)];
                zeroFlag = (diff == 0);
                signedFlag = (diff < 0);
                carryFlag = signedFlag;
                lastResult = diff;
                break;
            }
            case OpCode::JMP: {
                auto& d = std::get<ArithData>(instr.data);
                pc = static_cast<size_t>(d.dest);
                continue;
            }
            case OpCode::JE: {
                auto& d = std::get<ArithData>(instr.data);
                if (zeroFlag) {
                    pc = static_cast<size_t>(d.dest);
                    continue;
                }
                break;
            }
            case OpCode::JNE: {
                auto& d = std::get<ArithData>(instr.data);
                if (!zeroFlag) {
                    pc = static_cast<size_t>(d.dest);
                    continue;
                }
                break;
            }
            case OpCode::JG: {
                auto& d = std::get<ArithData>(instr.data);
                if (!zeroFlag && !signedFlag) {
                    pc = static_cast<size_t>(d.dest);
                    continue;
                }
                break;
            }
            case OpCode::JL: {
                auto& d = std::get<ArithData>(instr.data);
                if (signedFlag) {
                    pc = static_cast<size_t>(d.dest);
                    continue;
                }
                break;
            }
            case OpCode::JGE: {
                auto& d = std::get<ArithData>(instr.data);
                if (!signedFlag) {
                    pc = static_cast<size_t>(d.dest);
                    continue;
                }
                break;
            }
            case OpCode::JLE: {
                auto& d = std::get<ArithData>(instr.data);
                if (zeroFlag || signedFlag) {
                    pc = static_cast<size_t>(d.dest);
                    continue;
                }
                break;
            }
            case OpCode::PRINT: {
                auto& d = std::get<ArithData>(instr.data);
                if (static_cast<size_t>(d.dest) < rv.size()) {
                    std::cout << rv[static_cast<size_t>(d.dest)] << std::endl;
                } else {
                    std::cout << 0 << std::endl;
                }
                lastResult = (static_cast<size_t>(d.dest) < rv.size()) ? rv[static_cast<size_t>(d.dest)] : 0;
                break;
            }
            default:
                throw std::runtime_error("Unknown opcode");
        }
        pc++;
    }
    return lastResult;
}
