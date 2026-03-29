/*VM.cpp*//*շատ նման է CPU աշխատանքին*/
#include "VM.h"
#include <iostream>

VirtualMachine::VirtualMachine(SymbolTable& sym, size_t regCount) 
    : symbolTable(sym), values(sym.getValuesVector()), pc(0) {
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
                // Եթե dest-ը var ինդեքս է, ուրեմն store
                if (data.dest < values.size()) {
                    values[data.dest] = rv[data.right];
                    std::cout << "STORE var[" << (int)data.dest << "], r" << (int)data.right;
                    lastResult = rv[data.right];
                } else {
                    rv[data.dest] = values[data.right];
                    std::cout << "LOAD r" << (int)data.dest << ", [v" << (int)data.right << "]";
                    lastResult = rv[data.dest];
                }
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
                rv[data.dest] = rv[data.dest] + ((double)((data.value >> 16) & 0xFFFF) * 65536.0);
                std::cout << "LIH r" << (int)data.dest << ", " << ((data.value >> 16) & 0xFFFF);
                lastResult = rv[data.dest];
                break;
            }
        }
        
        std::cout << " = " << lastResult << "\n";
        pc++;
    }
    
    return lastResult;
}