#include "VM.h"
#include <iostream>

VirtualMachine::VirtualMachine(SymbolTable& sym, size_t regCount)
    : symbolTable(sym), values(sym.getValuesVector()), pc(0),
      zeroFlag(false), carryFlag(false), signedFlag(false) {
    rv.resize(regCount, 0.0);
}

void VirtualMachine::setRegister(size_t idx, double val) {
    if (idx >= rv.size()) rv.resize(idx + 1, 0.0);
    rv[idx] = val;
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

        switch (instr.opcode) {
            case OpCode::LI: {
                auto& d = std::get<LiData>(instr.data);
                if (d.dest >= rv.size()) rv.resize(d.dest + 1, 0.0);
                rv[d.dest] = (double)(int)d.value;
                lastResult = rv[d.dest];
                break;
            }
            case OpCode::LIL: {
                auto& d = std::get<LiData>(instr.data);
                if (d.dest >= rv.size()) rv.resize(d.dest + 1, 0.0);
                rv[d.dest] = (double)(d.value & 0xFFFF);
                lastResult = rv[d.dest];
                break;
            }
            case OpCode::LIH: {
                auto& d = std::get<LiData>(instr.data);
                if (d.dest >= rv.size()) rv.resize(d.dest + 1, 0.0);
                rv[d.dest] += (double)((d.value & 0xFFFF) * 65536);
                lastResult = rv[d.dest];
                break;
            }
            case OpCode::LOAD: {
                auto& d = std::get<ArithData>(instr.data);
                if (d.dest >= rv.size()) rv.resize(d.dest + 1, 0.0);
                while (d.right >= values.size()) values.push_back(0.0);
                rv[d.dest] = values[d.right];
                lastResult = rv[d.dest];
                break;
            }
            case OpCode::STORE: {
                auto& d = std::get<ArithData>(instr.data);
                while (d.dest >= values.size()) values.push_back(0.0);
                if (d.right >= rv.size()) rv.resize(d.right + 1, 0.0);
                values[d.dest] = rv[d.right];
                lastResult = values[d.dest];
                break;
            }
            case OpCode::ADD: {
                auto& d = std::get<ArithData>(instr.data);
                if (d.dest >= rv.size()) rv.resize(d.dest + 1, 0.0);
                rv[d.dest] = rv[d.left] + rv[d.right];
                lastResult = rv[d.dest];
                break;
            }
            case OpCode::SUB: {
                auto& d = std::get<ArithData>(instr.data);
                if (d.dest >= rv.size()) rv.resize(d.dest + 1, 0.0);
                rv[d.dest] = rv[d.left] - rv[d.right];
                lastResult = rv[d.dest];
                break;
            }
            case OpCode::MUL: {
                auto& d = std::get<ArithData>(instr.data);
                if (d.dest >= rv.size()) rv.resize(d.dest + 1, 0.0);
                rv[d.dest] = rv[d.left] * rv[d.right];
                lastResult = rv[d.dest];
                break;
            }
            case OpCode::DIV: {
                auto& d = std::get<ArithData>(instr.data);
                if (rv[d.right] == 0.0) throw std::runtime_error("Division by zero");
                if (d.dest >= rv.size()) rv.resize(d.dest + 1, 0.0);
                rv[d.dest] = rv[d.left] / rv[d.right];
                lastResult = rv[d.dest];
                break;
            }
            case OpCode::CMP: {
                auto& d = std::get<ArithData>(instr.data);
                double diff = rv[d.left] - rv[d.right];
                zeroFlag  = (diff == 0.0);
                signedFlag = (diff < 0.0);
                carryFlag  = signedFlag;
                lastResult = diff;
                break;
            }
            case OpCode::JMP: {
                auto& d = std::get<ArithData>(instr.data);
                pc = (size_t)d.dest;
                continue;
            }
            case OpCode::JE: {
                auto& d = std::get<ArithData>(instr.data);
                if (zeroFlag) { pc = (size_t)d.dest; continue; }
                break;
            }
            case OpCode::JNE: {
                auto& d = std::get<ArithData>(instr.data);
                if (!zeroFlag) { pc = (size_t)d.dest; continue; }
                break;
            }
            case OpCode::JG: {
                auto& d = std::get<ArithData>(instr.data);
                if (!zeroFlag && !signedFlag) { pc = (size_t)d.dest; continue; }
                break;
            }
            case OpCode::JL: {
                auto& d = std::get<ArithData>(instr.data);
                if (signedFlag) { pc = (size_t)d.dest; continue; }
                break;
            }
            case OpCode::JGE: {
                auto& d = std::get<ArithData>(instr.data);
                if (!signedFlag) { pc = (size_t)d.dest; continue; }
                break;
            }
            case OpCode::JLE: {
                auto& d = std::get<ArithData>(instr.data);
                if (zeroFlag || signedFlag) { pc = (size_t)d.dest; continue; }
                break;
            }
            case OpCode::PRINT: {
                auto& d = std::get<ArithData>(instr.data);
                if (d.dest < rv.size()) {
                    std::cout << rv[d.dest] << std::endl;
                } else {
                    std::cout << 0.0 << std::endl;
                }
                lastResult = (d.dest < rv.size()) ? rv[d.dest] : 0.0;
                break;
            }
            default:
                throw std::runtime_error("Unknown opcode");
        }
        pc++;
    }
    return lastResult;
}